#pragma once

#include <QDialog>
#include <QString>

namespace KLinkMonitorNative {

class HistoryDb;
class InterfaceMonitor;

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(HistoryDb &history,
                              InterfaceMonitor &monitor,
                              const QString &interfaceName,
                              QWidget *parent = nullptr);
};

} // namespace KLinkMonitorNative
