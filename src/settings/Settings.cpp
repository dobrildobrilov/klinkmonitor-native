#include "Settings.h"

namespace KLinkMonitorNative {

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("KLinkMonitor Native"), QStringLiteral("KLinkMonitor Native"))
{
}

int Settings::intervalMs() const
{
    return m_settings.value(QStringLiteral("monitor/intervalMs"), 5000).toInt();
}

void Settings::setIntervalMs(int value)
{
    m_settings.setValue(QStringLiteral("monitor/intervalMs"), value);
    emit changed();
}

bool Settings::notificationsEnabled() const
{
    return m_settings.value(QStringLiteral("notifications/enabled"), true).toBool();
}

void Settings::setNotificationsEnabled(bool value)
{
    m_settings.setValue(QStringLiteral("notifications/enabled"), value);
    emit changed();
}

QString Settings::selectedInterface() const
{
    return m_settings.value(QStringLiteral("monitor/interface"), QStringLiteral("auto")).toString();
}

void Settings::setSelectedInterface(const QString &value)
{
    m_settings.setValue(QStringLiteral("monitor/interface"), value);
    emit changed();
}

}
