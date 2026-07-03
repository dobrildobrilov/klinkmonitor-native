#include "InterfaceMonitor.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

namespace KLinkMonitorNative {

InterfaceMonitor::InterfaceMonitor(QObject *parent) : QObject(parent) {}

QStringList InterfaceMonitor::interfaces() const
{
    QDir dir(QStringLiteral("/sys/class/net"));
    QStringList names = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    names.removeAll(QStringLiteral("lo"));
    return names;
}
int InterfaceMonitor::sysfsType(const QString &name) const
{
    QFile file(QStringLiteral("/sys/class/net/%1/type").arg(name));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return -1;
    }

    return QString::fromUtf8(file.readAll()).trimmed().toInt();
}

bool InterfaceMonitor::hasWirelessSysfs(const QString &name) const
{
    return QFileInfo::exists(
        QStringLiteral("/sys/class/net/%1/wireless").arg(name)
    );
}

InterfaceKind InterfaceMonitor::kindForInterface(const QString &name) const
{
    const QString n = name.toLower();
    const int type = sysfsType(name);

    if (name == QStringLiteral("lo")) {
        return InterfaceKind::Unknown;
    }

    if (hasWirelessSysfs(name) ||
        n.startsWith(QStringLiteral("wl")) ||
        n.startsWith(QStringLiteral("wlan"))) {
        return InterfaceKind::Wifi;
    }

    // WireGuard usually appears as ARPHRD_NONE / 65534.
    // Example: link/none + "wireguard" in `ip -details link`.
    if (type == 65534 ||
        n.startsWith(QStringLiteral("wg")) ||
        n.contains(QStringLiteral("wireguard")) ||
        n.startsWith(QStringLiteral("stanga"))) {
        return InterfaceKind::WireGuard;
    }

    if (n.startsWith(QStringLiteral("tun")) ||
        n.startsWith(QStringLiteral("tap"))) {
        return InterfaceKind::Vpn;
    }

    if (n.startsWith(QStringLiteral("virbr")) ||
        n.startsWith(QStringLiteral("vnet")) ||
        n.startsWith(QStringLiteral("docker")) ||
        n.startsWith(QStringLiteral("br-"))) {
        return InterfaceKind::Virtual;
    }

    if (type == 1) {
        return InterfaceKind::Ethernet;
    }

    return InterfaceKind::Virtual;
}

QString InterfaceMonitor::operState(const QString &name) const
{
    QFile file(QStringLiteral("/sys/class/net/%1/operstate").arg(name));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QStringLiteral("unknown");
    return QString::fromUtf8(file.readAll()).trimmed();
}

QString InterfaceMonitor::ethernetSpeed(const QString &name) const
{
    QFile file(QStringLiteral("/sys/class/net/%1/speed").arg(name));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QStringLiteral("unknown");
    return QString::fromUtf8(file.readAll()).trimmed();
}

QString InterfaceMonitor::humanSpeed(const QString &speed)
{
    if (speed == QStringLiteral("10000")) return QStringLiteral("10 Gbps");
    if (speed == QStringLiteral("5000")) return QStringLiteral("5 Gbps");
    if (speed == QStringLiteral("2500")) return QStringLiteral("2.5 Gbps");
    if (speed == QStringLiteral("1000")) return QStringLiteral("1 Gbps");
    if (speed == QStringLiteral("100")) return QStringLiteral("100 Mbps");
    if (speed == QStringLiteral("10")) return QStringLiteral("10 Mbps");
    if (speed == QStringLiteral("down")) return QStringLiteral("Link Down");
    return speed.isEmpty() ? QStringLiteral("Unknown") : speed;
}

QString InterfaceMonitor::commandOutput(const QString &program, const QStringList &args) const
{
    QProcess process;
    process.start(program, args);
    if (!process.waitForFinished(1200)) {
        process.kill();
        return QString();
    }
    return QString::fromUtf8(process.readAllStandardOutput());
}

InterfaceInfo InterfaceMonitor::readEthernet(const QString &name) const
{
    InterfaceInfo info;
    info.name = name;
    info.kind = InterfaceKind::Ethernet;

    const QString state = operState(name);
    if (state != QStringLiteral("up")) {
        info.state = StatusState::Bad;
        info.shortText = QStringLiteral("Link Down");
        info.rawValue = QStringLiteral("down");
        info.displayValue = QStringLiteral("Link Down");
        info.tooltip = QStringLiteral("Interface: %1\nType: Ethernet\nStatus: Link Down").arg(name);
        return info;
    }

    const QString speed = ethernetSpeed(name);
    info.rawValue = speed;
    info.displayValue = humanSpeed(speed);
    info.shortText = info.displayValue;
    if (speed == QStringLiteral("10000") || speed == QStringLiteral("5000") || speed == QStringLiteral("2500") || speed == QStringLiteral("1000")) {
        info.state = StatusState::Good;
    } else if (speed == QStringLiteral("100")) {
        info.state = StatusState::Warning;
    } else {
        info.state = StatusState::Bad;
    }
    info.tooltip = QStringLiteral("Interface: %1\nType: Ethernet\nSpeed: %2").arg(name, info.displayValue);
    return info;
}

InterfaceInfo InterfaceMonitor::readWifi(const QString &name) const
{
    InterfaceInfo info;
    info.name = name;
    info.kind = InterfaceKind::Wifi;

    const QString state = operState(name);
    const QString out = commandOutput(QStringLiteral("iw"), {QStringLiteral("dev"), name, QStringLiteral("link")});
    const bool connected = out.contains(QStringLiteral("SSID:"));
    if (state != QStringLiteral("up") || !connected) {
        info.state = StatusState::Bad;
        info.rawValue = QStringLiteral("wifi:down");
        info.displayValue = QStringLiteral("Wi-Fi Disconnected");
        info.shortText = info.displayValue;
        info.tooltip = QStringLiteral("Interface: %1\nType: Wi-Fi\nStatus: Disconnected").arg(name);
        return info;
    }

    auto matchValue = [&out](const QString &pattern) -> QString {
        QRegularExpression re(pattern, QRegularExpression::MultilineOption);
        const auto m = re.match(out);
        return m.hasMatch() ? m.captured(1).trimmed() : QString();
    };
    const QString ssid = matchValue(QStringLiteral("SSID:\\s+(.+)"));
    const QString signal = matchValue(QStringLiteral("signal:\\s+(-?\\d+\\s+dBm)"));
    const QString tx = matchValue(QStringLiteral("tx bitrate:\\s+([0-9.]+\\s+\\S+/s)"));
    const QString rx = matchValue(QStringLiteral("rx bitrate:\\s+([0-9.]+\\s+\\S+/s)"));

    int signalValue = -50;
    if (!signal.isEmpty()) signalValue = signal.section(' ', 0, 0).toInt();
    if (signalValue >= -67) info.state = StatusState::Good;
    else if (signalValue >= -75) info.state = StatusState::Warning;
    else info.state = StatusState::Bad;

    info.rawValue = connected ? QStringLiteral("wifi:connected") : QStringLiteral("wifi:down");
    info.displayValue = tx.isEmpty() ? QStringLiteral("Wi-Fi Connected") : QStringLiteral("Wi-Fi %1").arg(tx);
    info.shortText = ssid.isEmpty() ? info.displayValue : QStringLiteral("%1 — %2").arg(ssid, tx.isEmpty() ? QStringLiteral("Connected") : tx);
    info.tooltip = QStringLiteral("Interface: %1\nType: Wi-Fi\nSSID: %2\nSignal: %3\nTX Rate: %4\nRX Rate: %5")
        .arg(name, ssid.isEmpty() ? QStringLiteral("Unknown") : ssid, signal.isEmpty() ? QStringLiteral("Unknown") : signal, tx.isEmpty() ? QStringLiteral("Unknown") : tx, rx.isEmpty() ? QStringLiteral("Unknown") : rx);
    return info;
}

InterfaceInfo InterfaceMonitor::readVirtualLike(const QString &name, InterfaceKind kind) const
{
    InterfaceInfo info;
    info.name = name;
    info.kind = kind;
    const QString state = operState(name);
    const bool up = (state == QStringLiteral("up") || state == QStringLiteral("unknown"));
    info.state = up ? StatusState::Good : StatusState::Bad;
    info.rawValue = up ? QStringLiteral("connected") : QStringLiteral("down");
    info.displayValue = up ? QStringLiteral("Connected") : QStringLiteral("Down");
    info.shortText = QStringLiteral("%1 %2").arg(kindName(kind), info.displayValue);
    info.tooltip = QStringLiteral("Interface: %1\nType: %2\nStatus: %3").arg(name, kindName(kind), info.displayValue);
    return info;
}

InterfaceInfo InterfaceMonitor::readInfo(const QString &name) const
{
    const auto kind = kindForInterface(name);
    if (kind == InterfaceKind::Ethernet) return readEthernet(name);
    if (kind == InterfaceKind::Wifi) return readWifi(name);
    return readVirtualLike(name, kind);
}

QString InterfaceMonitor::autoInterface() const
{
    const auto names = interfaces();
    for (const auto &name : names) {
        if (kindForInterface(name) == InterfaceKind::Ethernet && operState(name) == QStringLiteral("up")) return name;
    }
    for (const auto &name : names) {
        if (kindForInterface(name) == InterfaceKind::Wifi && operState(name) == QStringLiteral("up")) return name;
    }
    for (const auto &name : names) {
        if (kindForInterface(name) == InterfaceKind::Ethernet) return name;
    }
    return names.isEmpty() ? QString() : names.first();
}

QString InterfaceMonitor::kindName(InterfaceKind kind)
{
    switch (kind) {
    case InterfaceKind::Ethernet: return QStringLiteral("Ethernet");
    case InterfaceKind::Wifi: return QStringLiteral("Wi-Fi");
    case InterfaceKind::WireGuard: return QStringLiteral("WireGuard");
    case InterfaceKind::Vpn: return QStringLiteral("OpenVPN/TUN");
    case InterfaceKind::Virtual: return QStringLiteral("Virtual");
    default: return QStringLiteral("Unknown");
    }
}

QString InterfaceMonitor::stateIcon(StatusState state)
{
    switch (state) {
    case StatusState::Good: return QStringLiteral("🟢");
    case StatusState::Warning: return QStringLiteral("🟡");
    case StatusState::Bad: return QStringLiteral("🔴");
    }
    return QStringLiteral("⚪");
}

}
