#include "data/DatabaseManager.h"
#include "data/DatabaseWorker.h"
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QMetaObject>
#include <QUuid>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
    , m_mainConnectionName("db_main_" + QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_workerConnectionName("db_worker_" + QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_workerThread(new QThread(this))
    , m_worker(new DatabaseWorker)
{
    m_worker->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::finished,
            m_worker, &QObject::deleteLater);
    m_workerThread->start();
}

DatabaseManager::~DatabaseManager()
{
    flush();
    m_workerThread->quit();
    m_workerThread->wait(3000);
    m_worker = nullptr;

    if (m_db.isOpen()) m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_mainConnectionName);
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    QDir().mkpath(QFileInfo(dbPath).absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_mainConnectionName);
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "DatabaseManager: main DB open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL");
    q.exec("PRAGMA synchronous=NORMAL");
    q.exec("PRAGMA busy_timeout=3000");

    createTables();

    // BlockingQueuedConnection：主线程等待，worker 线程自己的 event loop 执行 slot，
    // 不依赖主线程 event loop——所以 event loop 未启动时也能正常工作
    bool ok = false;
    QMetaObject::invokeMethod(m_worker, "initialize", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, ok),
                              Q_ARG(QString, dbPath),
                              Q_ARG(QString, m_workerConnectionName));
    m_initialized = ok;
    return ok;
}

void DatabaseManager::saveData(const DeviceData& data)
{
    if (!m_initialized) return;
    QMetaObject::invokeMethod(m_worker, "saveData", Qt::QueuedConnection,
                              Q_ARG(DeviceData, data));
}

void DatabaseManager::saveAlarm(const AlarmEvent& event)
{
    if (!m_initialized) return;
    QMetaObject::invokeMethod(m_worker, "saveAlarm", Qt::QueuedConnection,
                              Q_ARG(AlarmEvent, event));
}

void DatabaseManager::flush()
{
    if (!m_initialized || !m_workerThread->isRunning()) return;

    QMetaObject::invokeMethod(m_worker, "flush", Qt::BlockingQueuedConnection);
}

void DatabaseManager::createTables()
{
    executeSchema(m_db);
}

bool DatabaseManager::executeSchema(QSqlDatabase& db)
{
    QSqlQuery q(db);
    bool ok = true;
    ok = q.exec(
        "CREATE TABLE IF NOT EXISTS device_data ("
        "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp   INTEGER NOT NULL,"
        "temperature REAL    NOT NULL,"
        "humidity    REAL    NOT NULL DEFAULT 0,"
        "pressure    REAL    NOT NULL,"
        "co2         REAL    NOT NULL DEFAULT 0,"
        "door_open   INTEGER NOT NULL DEFAULT 0,"
        "status_code TEXT    NOT NULL DEFAULT 'OK'"
        ")") && ok;
    ok = q.exec(
        "CREATE TABLE IF NOT EXISTS alarm_log ("
        "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp   INTEGER NOT NULL,"
        "channel     TEXT    NOT NULL,"
        "alarm_type  TEXT    NOT NULL,"
        "value       REAL    NOT NULL,"
        "limit_value REAL    NOT NULL,"
        "message     TEXT    NOT NULL"
        ")") && ok;
    ok = q.exec("CREATE INDEX IF NOT EXISTS idx_timestamp ON device_data(timestamp)") && ok;
    ok = q.exec("CREATE INDEX IF NOT EXISTS idx_alarm_timestamp ON alarm_log(timestamp)") && ok;
    if (!ok) {
        qWarning() << "DatabaseManager: create schema failed:" << q.lastError().text();
    }
    return ok;
}

QList<DeviceData> DatabaseManager::queryByTimeRange(
    const QDateTime& from, const QDateTime& to,
    int offset, int limit)
{
    QList<DeviceData> result;
    if (!m_db.isOpen()) return result;

    QSqlQuery query(m_db);
    QString sql = "SELECT timestamp, temperature, humidity, pressure, co2, door_open, status_code "
                  "FROM device_data "
                  "WHERE timestamp BETWEEN ? AND ? "
                  "ORDER BY timestamp ASC";
    if (limit > 0)
        sql += " LIMIT ? OFFSET ?";

    query.prepare(sql);
    query.addBindValue(from.toMSecsSinceEpoch());
    query.addBindValue(to.toMSecsSinceEpoch());
    if (limit > 0) {
        query.addBindValue(limit);
        query.addBindValue(qMax(0, offset));
    }
    if (!query.exec()) {
        qWarning() << "DatabaseManager: queryByTimeRange failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        DeviceData data;
        data.timestamp   = QDateTime::fromMSecsSinceEpoch(query.value(0).toLongLong());
        data.temperature = query.value(1).toDouble();
        data.humidity    = query.value(2).toDouble();
        data.pressure    = query.value(3).toDouble();
        data.co2         = query.value(4).toDouble();
        data.doorOpen    = query.value(5).toInt() == 1;
        data.statusCode  = query.value(6).toString();
        data.isValid     = true;
        result.append(data);
    }
    return result;
}

int DatabaseManager::countByTimeRange(
    const QDateTime& from, const QDateTime& to)
{
    if (!m_db.isOpen()) return 0;

    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM device_data WHERE timestamp BETWEEN ? AND ?");
    query.addBindValue(from.toMSecsSinceEpoch());
    query.addBindValue(to.toMSecsSinceEpoch());
    if (!query.exec()) {
        qWarning() << "DatabaseManager: countByTimeRange failed:" << query.lastError().text();
        return 0;
    }

    if (query.next())
        return query.value(0).toInt();
    return 0;
}

QList<AlarmEvent> DatabaseManager::queryAlarmsByTimeRange(
    const QDateTime& from, const QDateTime& to)
{
    QList<AlarmEvent> result;
    if (!m_db.isOpen()) return result;

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT timestamp, channel, alarm_type, value, limit_value, message "
        "FROM alarm_log "
        "WHERE timestamp BETWEEN ? AND ? "
        "ORDER BY timestamp DESC"
        );
    query.addBindValue(from.toMSecsSinceEpoch());
    query.addBindValue(to.toMSecsSinceEpoch());
    if (!query.exec()) {
        qWarning() << "DatabaseManager: queryAlarmsByTimeRange failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        AlarmEvent evt;
        evt.timestamp = QDateTime::fromMSecsSinceEpoch(query.value(0).toLongLong());
        evt.channel   = query.value(1).toString();
        evt.type      = (query.value(2).toString() == "HIGH")
                            ? AlarmEvent::Type::HighLimit
                            : AlarmEvent::Type::LowLimit;
        evt.value     = query.value(3).toDouble();
        evt.limit     = query.value(4).toDouble();
        evt.message   = query.value(5).toString();
        result.append(evt);
    }
    return result;
}

bool DatabaseManager::exportToCsv(
    const QString& filePath,
    const QDateTime& from,
    const QDateTime& to)
{
    auto dataList = queryByTimeRange(from, to);
    if (dataList.isEmpty()) return false;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF";
    out << "时间,温度(℃),湿度(%),压力(MPa),CO2(ppm),门状态,状态\n";
    for (const auto& data : dataList) {
        out << data.timestamp.toString("yyyy-MM-dd hh:mm:ss") << ","
            << QString::number(data.temperature, 'f', 1) << ","
            << QString::number(data.humidity,    'f', 1) << ","
            << QString::number(data.pressure,    'f', 4) << ","
            << QString::number(data.co2,         'f', 0) << ","
            << (data.doorOpen ? "开" : "关") << ","
            << data.statusCode << "\n";
    }
    return true;
}
