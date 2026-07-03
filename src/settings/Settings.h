#pragma once

#include <QObject>
#include <QSettings>

namespace KLinkMonitorNative {

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);

    int intervalMs() const;
    void setIntervalMs(int value);

    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool value);

    QString selectedInterface() const;
    void setSelectedInterface(const QString &value);

signals:
    void changed();

private:
    QSettings m_settings;
};
}
