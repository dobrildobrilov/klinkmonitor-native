#include "Tray.h"

#include <QApplication>
#include <QColor>
#include <QProcess>

#include "Version.h"
#include "database/HistoryDb.h"
#include "dialogs/AboutDialog.h"
#include "dialogs/HistoryDialog.h"
#include "dialogs/SettingsDialog.h"
#include "dialogs/StatisticsDialog.h"
#include "icons/IconFactory.h"
#include "settings/Settings.h"

namespace KLinkMonitorNative {

Tray::Tray(InterfaceMonitor &monitor, HistoryDb &history, Settings &settings, QObject *parent)
    : QObject(parent)
    , m_monitor(monitor)
    , m_history(history)
    , m_settings(settings)
    , m_sni(new KStatusNotifierItem(QStringLiteral(KLINKMONITOR_COMPONENT_NAME), this))
    , m_menu(new QMenu())
{
    m_emptyPixmap = QPixmap(64, 64);
    m_emptyPixmap.fill(Qt::transparent);
    m_currentPixmap = IconFactory::ethernetIcon(QColor(QStringLiteral("#7f8c8d"))).pixmap(64, 64);

    m_sni->setTitle(QStringLiteral("KLinkMonitor Native"));
    m_sni->setCategory(KStatusNotifierItem::SystemServices);
    m_sni->setStatus(KStatusNotifierItem::Active);
    setTrayPixmap(m_currentPixmap);

    buildMenu();
    m_sni->setContextMenu(m_menu);

    connect(m_menu, &QMenu::aboutToShow, this, &Tray::buildMenu);
    connect(&m_timer, &QTimer::timeout, this, &Tray::updateStatus);
    connect(&m_blinkTimer, &QTimer::timeout, this, &Tray::blinkIcon);
    connect(&m_settings, &Settings::changed, this, &Tray::applySettings);

    m_timer.start(m_settings.intervalMs());
    m_blinkTimer.start(600);
    updateStatus();
}

void Tray::show()
{
    m_sni->setStatus(KStatusNotifierItem::Active);
}

void Tray::applySettings()
{
    m_timer.setInterval(m_settings.intervalMs());
    buildMenu();
    updateStatus();
}

void Tray::buildMenu()
{
    m_menu->clear();
    buildInterfacesMenu();
    m_menu->addSeparator();

    QAction *history = m_menu->addAction(QStringLiteral("History"));
    connect(history, &QAction::triggered, this, &Tray::showHistory);

    QAction *stats = m_menu->addAction(QStringLiteral("Statistics"));
    connect(stats, &QAction::triggered, this, &Tray::showStatistics);

    QAction *settings = m_menu->addAction(QStringLiteral("Settings"));
    connect(settings, &QAction::triggered, this, &Tray::showSettings);

    QAction *about = m_menu->addAction(QStringLiteral("About KLinkMonitor Native"));
    connect(about, &QAction::triggered, this, []() { showAboutDialog(); });

    m_menu->addSeparator();
    QAction *exit = m_menu->addAction(QStringLiteral("Exit KLinkMonitor Native"));
    connect(exit, &QAction::triggered, this, &Tray::exitApplication);
}

void Tray::buildInterfacesMenu()
{
    m_interfacesMenu = m_menu->addMenu(QStringLiteral("Interfaces"));
    m_interfaceGroup = new QActionGroup(m_interfacesMenu);
    m_interfaceGroup->setExclusive(true);

    const QString selected = m_settings.selectedInterface();
    const QString autoIface = m_monitor.autoInterface();
    QAction *autoAction = m_interfacesMenu->addAction(QStringLiteral("Auto (%1)").arg(autoIface.isEmpty() ? QStringLiteral("none") : autoIface));
    autoAction->setCheckable(true);
    autoAction->setChecked(selected == QStringLiteral("auto"));
    m_interfaceGroup->addAction(autoAction);
    connect(autoAction, &QAction::triggered, this, &Tray::setAutoMode);
    m_interfacesMenu->addSeparator();

    struct Group { QString title; InterfaceKind kind; };
    const QList<Group> groups = {
        {QStringLiteral("Ethernet"), InterfaceKind::Ethernet},
        {QStringLiteral("Wi-Fi"), InterfaceKind::Wifi},
        {QStringLiteral("VPN / WireGuard"), InterfaceKind::WireGuard},
        {QStringLiteral("OpenVPN / TUN"), InterfaceKind::Vpn},
        {QStringLiteral("Virtual"), InterfaceKind::Virtual},
    };

    const QStringList names = m_monitor.interfaces();
    for (const auto &g : groups) {
        bool headerAdded = false;
        for (const auto &name : names) {
            const auto info = m_monitor.readInfo(name);
            const bool groupMatch = (g.kind == InterfaceKind::WireGuard)
                ? (info.kind == InterfaceKind::WireGuard)
                : (info.kind == g.kind);
            if (!groupMatch) continue;
            if (!headerAdded) {
                auto *hdr = m_interfacesMenu->addAction(g.title);
                hdr->setEnabled(false);
                headerAdded = true;
            }
            QAction *action = m_interfacesMenu->addAction(QStringLiteral("%1 %2 — %3").arg(InterfaceMonitor::stateIcon(info.state), name, info.shortText));
            action->setCheckable(true);
            action->setChecked(selected == name);
            m_interfaceGroup->addAction(action);
            connect(action, &QAction::triggered, this, [this, name]() { setInterface(name); });
        }
        if (headerAdded) m_interfacesMenu->addSeparator();
    }
}

QString Tray::effectiveInterface() const
{
    const QString selected = m_settings.selectedInterface();
    if (selected == QStringLiteral("auto")) return m_monitor.autoInterface();
    return selected;
}

QColor Tray::colorForState(StatusState state) const
{
    switch (state) {
    case StatusState::Good: return QColor(QStringLiteral("#2ecc71"));
    case StatusState::Warning: return QColor(QStringLiteral("#f1c40f"));
    case StatusState::Bad: return QColor(QStringLiteral("#e74c3c"));
    }
    return QColor(QStringLiteral("#7f8c8d"));
}

QIcon Tray::iconForInfo(const InterfaceInfo &info) const
{
    return IconFactory::iconForKind(info.kind, colorForState(info.state));
}

QPixmap Tray::pixmapForInfo(const InterfaceInfo &info) const
{
    return iconForInfo(info).pixmap(64, 64);
}

void Tray::setTrayPixmap(const QPixmap &pixmap)
{
    m_sni->setIconByPixmap(pixmap);
}

void Tray::updateBlinkState(const InterfaceInfo &info)
{
    m_currentPixmap = pixmapForInfo(info);
    m_shouldBlink = (info.state == StatusState::Warning || info.state == StatusState::Bad);

    if (!m_shouldBlink) {
        m_blinkVisible = true;
        setTrayPixmap(m_currentPixmap);
        return;
    }

    if (m_blinkVisible) {
        setTrayPixmap(m_currentPixmap);
    } else {
        setTrayPixmap(m_emptyPixmap);
    }
}

void Tray::blinkIcon()
{
    if (!m_shouldBlink) {
        return;
    }

    m_blinkVisible = !m_blinkVisible;
    setTrayPixmap(m_blinkVisible ? m_currentPixmap : m_emptyPixmap);
}

QString Tray::displayForRaw(const InterfaceInfo &info, const QString &raw) const
{
    if (info.kind == InterfaceKind::Ethernet) return InterfaceMonitor::humanSpeed(raw);
    if (raw == QStringLiteral("connected")) return QStringLiteral("Connected");
    if (raw == QStringLiteral("down")) return QStringLiteral("Down");
    if (raw == QStringLiteral("wifi:connected")) return QStringLiteral("Connected");
    if (raw == QStringLiteral("wifi:down")) return QStringLiteral("Disconnected");
    return raw;
}

QString Tray::eventForChange(const InterfaceInfo &info, const QString &previous, const QString &current) const
{
    if (info.kind == InterfaceKind::Ethernet) {
        if (current == QStringLiteral("100") && (previous == QStringLiteral("1000") || previous == QStringLiteral("2500") || previous == QStringLiteral("5000") || previous == QStringLiteral("10000"))) return QStringLiteral("Speed dropped");
        if ((current == QStringLiteral("1000") || current == QStringLiteral("2500") || current == QStringLiteral("5000") || current == QStringLiteral("10000")) && previous == QStringLiteral("100")) return QStringLiteral("Speed restored");
        if (current == QStringLiteral("down")) return QStringLiteral("Link Down");
        if (previous == QStringLiteral("down")) return QStringLiteral("Link Up");
        return QStringLiteral("Speed changed");
    }
    if (current.contains(QStringLiteral("down")) || current == QStringLiteral("down")) return QStringLiteral("Disconnected");
    if (previous.contains(QStringLiteral("down")) || previous == QStringLiteral("down")) return QStringLiteral("Connected");
    return QStringLiteral("Status changed");
}

QString Tray::severityForChange(const InterfaceInfo &info, const QString &, const QString &) const
{
    if (info.state == StatusState::Bad) return QStringLiteral("critical");
    if (info.state == StatusState::Warning) return QStringLiteral("warning");
    return QStringLiteral("info");
}

void Tray::sendNotification(const QString &text, const QString &severity)
{
    if (!m_settings.notificationsEnabled()) return;

    QString urgency = QStringLiteral("normal");
    if (severity == QStringLiteral("critical")) urgency = QStringLiteral("critical");
    else if (severity == QStringLiteral("warning")) urgency = QStringLiteral("normal");

    QProcess::startDetached(QStringLiteral("notify-send"), {
        QStringLiteral("-u"), urgency,
        QStringLiteral("KLinkMonitor Native"), text
    });
}

void Tray::processInterfaceChange(const InterfaceInfo &info)
{
    if (info.name.isEmpty() || info.rawValue.isEmpty()) {
        return;
    }

    const QString previous = m_lastValues.value(info.name);

    // First observation is a baseline, not a history event.
    // History should contain real changes only.
    if (previous.isEmpty()) {
        m_lastValues.insert(info.name, info.rawValue);
        return;
    }

    if (previous != info.rawValue) {
        const QString event = eventForChange(info, previous, info.rawValue);
        const QString severity = severityForChange(info, previous, info.rawValue);
        const QString oldDisplay = displayForRaw(info, previous);
        const QString newDisplay = displayForRaw(info, info.rawValue);

        m_history.logEvent(info.name, event, oldDisplay, newDisplay, severity);
        sendNotification(QStringLiteral("%1: %2 → %3").arg(info.name, oldDisplay, newDisplay), severity);
    }

    m_lastValues.insert(info.name, info.rawValue);
}

void Tray::updateStatus()
{
    const QStringList names = m_monitor.interfaces();
    if (names.isEmpty()) {
        m_sni->setToolTipTitle(QStringLiteral("KLinkMonitor Native"));
        m_sni->setToolTipSubTitle(QStringLiteral("No network interfaces found"));
        InterfaceInfo empty;
        empty.kind = InterfaceKind::Ethernet;
        empty.state = StatusState::Bad;
        updateBlinkState(empty);
        return;
    }

    const QString iface = effectiveInterface();
    InterfaceInfo effectiveInfo;
    bool haveEffectiveInfo = false;

    // Poll every interface so History records Wi-Fi/VPN/Ethernet changes even
    // when Auto currently displays a different interface in the tray.
    for (const QString &name : names) {
        const InterfaceInfo info = m_monitor.readInfo(name);
        processInterfaceChange(info);

        if (name == iface) {
            effectiveInfo = info;
            haveEffectiveInfo = true;
        }
    }

    if (!haveEffectiveInfo) {
        effectiveInfo = m_monitor.readInfo(iface.isEmpty() ? names.first() : iface);
        processInterfaceChange(effectiveInfo);
    }

    updateBlinkState(effectiveInfo);

    const QString health = effectiveInfo.state == StatusState::Good
        ? QStringLiteral("Excellent")
        : (effectiveInfo.state == StatusState::Warning ? QStringLiteral("Warning") : QStringLiteral("Problem"));

    const QString tooltip = QStringLiteral("KLinkMonitor Native\n\n%1\n\nHealth: %2")
        .arg(effectiveInfo.tooltip, health);
    m_sni->setToolTipTitle(QStringLiteral("KLinkMonitor Native"));
    m_sni->setToolTipSubTitle(tooltip);
}

void Tray::setInterface(const QString &interfaceName)
{
    m_settings.setSelectedInterface(interfaceName);
}

void Tray::setAutoMode()
{
    m_settings.setSelectedInterface(QStringLiteral("auto"));
}

void Tray::showHistory()
{
    HistoryDialog dlg(m_history, QString(), nullptr);
    dlg.exec();
}

void Tray::showStatistics()
{
    StatisticsDialog dlg(m_history, m_monitor, effectiveInterface(), nullptr);
    dlg.exec();
}

void Tray::showSettings()
{
    SettingsDialog dlg(m_settings, nullptr);
    dlg.exec();
}

void Tray::exitApplication()
{
    qApp->quit();
}

}
