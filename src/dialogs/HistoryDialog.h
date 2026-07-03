#pragma once

#include <QDialog>
#include <QString>

namespace KLinkMonitorNative {
class HistoryDb;

class HistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDialog(HistoryDb &history, const QString &interfaceName = QString(), QWidget *parent = nullptr);
private:
    HistoryDb &m_history;
    QString m_interfaceName;
    QString eventLabel(const QString &event, const QString &severity) const;
};
}
