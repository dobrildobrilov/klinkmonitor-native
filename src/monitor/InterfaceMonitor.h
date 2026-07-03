#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace KLinkMonitorNative {

enum class InterfaceKind { Ethernet, Wifi, WireGuard, Vpn, Virtual, Unknown };
enum class StatusState { Good, Warning, Bad };

struct InterfaceInfo {
    QString name;
    InterfaceKind kind = InterfaceKind::Unknown;
    StatusState state = StatusState::Bad;
    QString shortText;
    QString tooltip;
    QString rawValue;
    QString displayValue;
};

class InterfaceMonitor : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceMonitor(QObject *parent = nullptr);

    QStringList interfaces() const;
    InterfaceKind kindForInterface(const QString &name) const;
    InterfaceInfo readInfo(const QString &name) const;
    QString autoInterface() const;

    static QString kindName(InterfaceKind kind);
    static QString stateIcon(StatusState state);
    static QString humanSpeed(const QString &speed);

private:
    QString operState(const QString &name) const;
    QString ethernetSpeed(const QString &name) const;
    int sysfsType(const QString &name) const;
    bool hasWirelessSysfs(const QString &name) const;
    QString commandOutput(const QString &program, const QStringList &args) const;
    InterfaceInfo readEthernet(const QString &name) const;
    InterfaceInfo readWifi(const QString &name) const;
    InterfaceInfo readVirtualLike(const QString &name, InterfaceKind kind) const;
};
}
