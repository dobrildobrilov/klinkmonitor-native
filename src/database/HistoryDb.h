#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QVector>

namespace KLinkMonitorNative {

struct EventRecord {
    QString timestamp;
    QString interfaceName;
    QString event;
    QString oldValue;
    QString newValue;
    QString severity;
};

class HistoryDb : public QObject
{
    Q_OBJECT
public:
    explicit HistoryDb(QObject *parent = nullptr);

    void logEvent(const QString &interfaceName, const QString &event, const QString &oldValue, const QString &newValue, const QString &severity);
    QVector<EventRecord> events(const QString &interfaceName = QString(), int limit = 500) const;
    void clearEvents();

    int countProblems(const QString &interfaceName = QString(), int days = -1) const;
    int countSpeedDrops(const QString &interfaceName = QString(), int days = -1) const;
    int countLinkDown(const QString &interfaceName = QString(), int days = -1) const;
    QString lastProblem(const QString &interfaceName = QString()) const;
    QString firstSeen(const QString &interfaceName = QString()) const;

private:
    QString dbPath() const;
    void initialize();
    QString whereScope(const QString &interfaceName, int days, QVariantList &binds) const;
};
}
