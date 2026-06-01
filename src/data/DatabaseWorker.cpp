#include "data/DatabaseWorker.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

DatabaseWorker::DatabaseWorker(QObject* parent)
    : QObject(parent)
{
}

DatabaseWorker::~DatabaseWorker()
{
    flush();
    if (m_flushTimer) {
        m_flushTimer->stop();
    }
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_db = QSqlDatabase();
    if (!m_connectionName.isEmpty()) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DatabaseWorker::initialize(const QString& dbPath, const QString& connectionName)
{
    m_dbPath = dbPath;
    m_connectionName = connectionName;
    QDir().mkpath(QFileInfo(dbPath).absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning() << "DatabaseWorker: DB open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery q(m_db);
    q.exec("PRAGMA journal_mode=WAL");
    q.exec("PRAGMA synchronous=NORMAL");
    q.exec("PRAGMA busy_timeout=3000");

    if (!createTables()) {
        return false;
    }

    if (!m_flushTimer) {
        m_flushTimer = new QTimer(this);
        m_flushTimer->setInterval(1000);
        m_flushTimer->setSingleShot(false);
        connect(m_flushTimer, &QTimer::timeout, this, &DatabaseWorker::flush);
    }
    m_flushTimer->start();
    return true;
}

bool DatabaseWorker::createTables()
{
    QSqlQuery q(m_db);
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
        qWarning() << "DatabaseWorker: create schema failed:" << q.lastError().text();
    }
    return ok;
}

void DatabaseWorker::saveData(const DeviceData& data)
{
    if (!data.isValid || !m_db.isOpen()) {
        return;
    }

    m_dataBuffer.append(data);
    if (m_dataBuffer.size() >= BATCH_SIZE) {
        flushData();
    }
}

void DatabaseWorker::saveAlarm(const AlarmEvent& event)
{
    if (!m_db.isOpen()) {
        return;
    }

    m_alarmBuffer.append(event);
    if (m_alarmBuffer.size() >= ALARM_BATCH_SIZE) {
        flushAlarms();
    }
}

void DatabaseWorker::flush()
{
    flushData();
    flushAlarms();
}

bool DatabaseWorker::flushData()
{
    if (m_dataBuffer.isEmpty() || !m_db.isOpen()) {
        return true;
    }

    QSqlQuery q(m_db);
    if (!m_db.transaction()) {
        qWarning() << "DatabaseWorker: begin device_data transaction failed:"
                   << m_db.lastError().text();
        return false;
    }

    q.prepare(
        "INSERT INTO device_data "
        "(timestamp, temperature, humidity, pressure, co2, door_open, status_code) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)");

    for (const auto& d : m_dataBuffer) {
        q.bindValue(0, d.timestamp.toMSecsSinceEpoch());
        q.bindValue(1, d.temperature);
        q.bindValue(2, d.humidity);
        q.bindValue(3, d.pressure);
        q.bindValue(4, d.co2);
        q.bindValue(5, d.doorOpen ? 1 : 0);
        q.bindValue(6, d.statusCode);

        if (!q.exec()) {
            qWarning() << "DatabaseWorker: insert device_data failed:" << q.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        qWarning() << "DatabaseWorker: commit device_data failed:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    m_dataBuffer.clear();
    return true;
}

bool DatabaseWorker::flushAlarms()
{
    if (m_alarmBuffer.isEmpty() || !m_db.isOpen()) {
        return true;
    }

    QSqlQuery q(m_db);
    if (!m_db.transaction()) {
        qWarning() << "DatabaseWorker: begin alarm_log transaction failed:"
                   << m_db.lastError().text();
        return false;
    }

    q.prepare(
        "INSERT INTO alarm_log "
        "(timestamp, channel, alarm_type, value, limit_value, message) "
        "VALUES (?, ?, ?, ?, ?, ?)");

    for (const auto& e : m_alarmBuffer) {
        q.bindValue(0, e.timestamp.toMSecsSinceEpoch());
        q.bindValue(1, e.channel);
        q.bindValue(2, e.type == AlarmEvent::Type::HighLimit ? "HIGH" : "LOW");
        q.bindValue(3, e.value);
        q.bindValue(4, e.limit);
        q.bindValue(5, e.message);

        if (!q.exec()) {
            qWarning() << "DatabaseWorker: insert alarm_log failed:" << q.lastError().text();
            m_db.rollback();
            return false;
        }
    }

    if (!m_db.commit()) {
        qWarning() << "DatabaseWorker: commit alarm_log failed:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }

    m_alarmBuffer.clear();
    return true;
}
