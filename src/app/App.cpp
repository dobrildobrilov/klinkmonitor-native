#include "App.h"

#include "database/HistoryDb.h"
#include "monitor/InterfaceMonitor.h"
#include "settings/Settings.h"
#include "tray/Tray.h"

namespace KLinkMonitorNative {

App::App(QObject *parent)
    : QObject(parent)
    , m_settings(std::make_unique<Settings>())
    , m_history(std::make_unique<HistoryDb>())
    , m_monitor(std::make_unique<InterfaceMonitor>())
    , m_tray(std::make_unique<Tray>(*m_monitor, *m_history, *m_settings))
{
    m_tray->show();
}

App::~App() = default;

}
