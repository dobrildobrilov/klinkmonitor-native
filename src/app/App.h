#pragma once

#include <QObject>
#include <memory>

namespace KLinkMonitorNative {
class Tray;
class InterfaceMonitor;
class HistoryDb;
class Settings;

class App : public QObject
{
    Q_OBJECT
public:
    explicit App(QObject *parent = nullptr);
    ~App() override;

private:
    std::unique_ptr<Settings> m_settings;
    std::unique_ptr<HistoryDb> m_history;
    std::unique_ptr<InterfaceMonitor> m_monitor;
    std::unique_ptr<Tray> m_tray;
};
}
