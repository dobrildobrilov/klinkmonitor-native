#include "HistoryDb.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QVariant>

namespace KLinkMonitorNative {

HistoryDb::HistoryDb(QObject *parent) : QObject(parent)
{
    initialize();
}

QString HistoryDb::dbPath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    if (dir.isEmpty()) {
	    dir = QDir::homePath() + QStringLiteral("/.local/share/klinkmonitor-native");
    }
    QDir().mkpath(dir);
    return dir + QStringLiteral("/history.sqlite3");
}

void HistoryDb::initialize()
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("history"));
    db.setDatabaseName(dbPath());
    if (!db.open()) return;

    QSqlQuery q(db);
    q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS events ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp TEXT NOT NULL,"
        "interface TEXT,"
        "event TEXT,"
        "old_value TEXT,"
        "new_value TEXT,"
        "severity TEXT"
        ")"
    ));
}

void HistoryDb::logEvent(const QString &interfaceName, const QString &event, const QString &oldValue, const QString &newValue, const QString &severity)
{
    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("history"));
    if (!db.isOpen() && !db.open()) return;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("INSERT INTO events(timestamp, interface, event, old_value, new_value, severity) VALUES(:t,:i,:e,:o,:n,:s)"));
    q.bindValue(QStringLiteral(":t"), QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(QStringLiteral(":i"), interfaceName);
    q.bindValue(QStringLiteral(":e"), event);
    q.bindValue(QStringLiteral(":o"), oldValue);
    q.bindValue(QStringLiteral(":n"), newValue);
    q.bindValue(QStringLiteral(":s"), severity);
    q.exec();
}

QVector<EventRecord> HistoryDb::events(const QString &interfaceName, int limit) const
{
    QVector<EventRecord> rows;
    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("history"));
    if (!db.isOpen() && !db.open()) return rows;

    QSqlQuery q(db);
    if (interfaceName.isEmpty()) {
        q.prepare(QStringLiteral("SELECT timestamp, interface, event, old_value, new_value, severity FROM events ORDER BY id DESC LIMIT :limit"));
    } else {
        q.prepare(QStringLiteral("SELECT timestamp, interface, event, old_value, new_value, severity FROM events WHERE interface=:iface ORDER BY id DESC LIMIT :limit"));
        q.bindValue(QStringLiteral(":iface"), interfaceName);
    }
    q.bindValue(QStringLiteral(":limit"), limit);
    if (!q.exec()) return rows;
    while (q.next()) {
        EventRecord r;
        r.timestamp = q.value(0).toString();
        r.interfaceName = q.value(1).toString();
        r.event = q.value(2).toString();
        r.oldValue = q.value(3).toString();
        r.newValue = q.value(4).toString();
        r.severity = q.value(5).toString();
        rows.push_back(r);
    }
    return rows;
}

void HistoryDb::clearEvents()
{
    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("history"));
    if (!db.isOpen() && !db.open()) return;
    QSqlQuery(QStringLiteral("DELETE FROM events"), db).exec();
}

QString HistoryDb::whereScope(const QString &interfaceName, int days, QVariantList &binds) const
{
    QStringList parts;
    if (!interfaceName.isEmpty()) {
        parts << QStringLiteral("interface=?");
        binds << interfaceName;
    }
    if (days >= 0) {
        parts << QStringLiteral("timestamp>=?");
        binds << QDateTime::currentDateTime().addDays(-days).toString(Qt::ISODate);
    }
    return parts.isEmpty() ? QString() : QStringLiteral(" WHERE ") + parts.join(QStringLiteral(" AND "));
}

static int scalarCount(const QString &sql, const QVariantList &binds)
{
    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("history"));
    if (!db.isOpen() && !db.open()) return 0;
    QSqlQuery q(db);
    q.prepare(sql);
    for (const auto &b : binds) q.addBindValue(b);
    if (!q.exec() || !q.next()) return 0;
    return q.value(0).toInt();
}

int HistoryDb::countProblems(const QString &interfaceName, int days) const
{
    QVariantList binds;
    QString where = whereScope(interfaceName, days, binds);
    if (where.isEmpty()) where = QStringLiteral(" WHERE severity IN ('warning','critical')");
    else where += QStringLiteral(" AND severity IN ('warning','critical')");
    return scalarCount(QStringLiteral("SELECT COUNT(*) FROM events") + where, binds);
}

int HistoryDb::countSpeedDrops(const QString &interfaceName, int days) const
{
    QVariantList binds;
    QString where = whereScope(interfaceName, days, binds);
    if (where.isEmpty()) where = QStringLiteral(" WHERE event='Speed dropped'");
    else where += QStringLiteral(" AND event='Speed dropped'");
    return scalarCount(QStringLiteral("SELECT COUNT(*) FROM events") + where, binds);
}

int HistoryDb::countLinkDown(const QString &interfaceName, int days) const
{
    QVariantList binds;
    QString where = whereScope(interfaceName, days, binds);
    if (where.isEmpty()) where = QStringLiteral(" WHERE event='Link Down'");
    else where += QStringLiteral(" AND event='Link Down'");
    return scalarCount(QStringLiteral("SELECT COUNT(*) FROM events") + where, binds);
}

QString HistoryDb::lastProblem(const QString &interfaceName) const
{
    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("history"));
    if (!db.isOpen() && !db.open()) return QStringLiteral("None");
    QSqlQuery q(db);
    if (interfaceName.isEmpty()) {
        q.prepare(QStringLiteral("SELECT timestamp FROM events WHERE severity IN ('warning','critical') ORDER BY id DESC LIMIT 1"));
    } else {
        q.prepare(QStringLiteral("SELECT timestamp FROM events WHERE interface=:i AND severity IN ('warning','critical') ORDER BY id DESC LIMIT 1"));
        q.bindValue(QStringLiteral(":i"), interfaceName);
    }
    if (!q.exec() || !q.next()) return QStringLiteral("None");
    return q.value(0).toString();
}

QString HistoryDb::firstSeen(const QString &interfaceName) const
{
    QSqlDatabase db = QSqlDatabase::database(QStringLiteral("history"));
    if (!db.isOpen() && !db.open()) return QStringLiteral("None");
    QSqlQuery q(db);
    if (interfaceName.isEmpty()) {
        q.prepare(QStringLiteral("SELECT timestamp FROM events ORDER BY id ASC LIMIT 1"));
    } else {
        q.prepare(QStringLiteral("SELECT timestamp FROM events WHERE interface=:i ORDER BY id ASC LIMIT 1"));
        q.bindValue(QStringLiteral(":i"), interfaceName);
    }
    if (!q.exec() || !q.next()) return QStringLiteral("None");
    return q.value(0).toString();
}

}
