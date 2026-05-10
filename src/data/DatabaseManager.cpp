#include "data/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{}

DatabaseManager::~DatabaseManager()
{
    // 剩余缓冲数据写入
    if (!m_buffer.isEmpty() && m_db.isOpen()) {
        QSqlQuery query(m_db);
        m_db.transaction();
        for (const auto& data : m_buffer) {
            query.prepare(
                "INSERT INTO device_data "
                "(timestamp, temperature, pressure, status_code) "
                "VALUES (?, ?, ?, ?)"
                );
            query.addBindValue(data.timestamp.toMSecsSinceEpoch());
            query.addBindValue(data.temperature);
            query.addBindValue(data.pressure);
            query.addBindValue(data.statusCode);
            query.exec();
        }
        m_db.commit();
    }
    if (m_db.isOpen()) m_db.close();
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    // 确保目录存在
    QDir().mkpath(QFileInfo(dbPath).absolutePath());

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "数据库打开失败:" << m_db.lastError().text();
        return false;
    }
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // 采集数据表
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS device_data ("
        "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp   INTEGER NOT NULL,"
        "temperature REAL    NOT NULL,"
        "pressure    REAL    NOT NULL,"
        "status_code TEXT    NOT NULL DEFAULT 'OK'"
        ")"
        );
    if (!ok) {
        qWarning() << "建表失败:" << query.lastError().text();
        return false;
    }

    // 报警日志表
    query.exec(
        "CREATE TABLE IF NOT EXISTS alarm_log ("
        "id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp   INTEGER NOT NULL,"
        "channel     TEXT    NOT NULL,"
        "alarm_type  TEXT    NOT NULL,"
        "value       REAL    NOT NULL,"
        "limit_value REAL    NOT NULL,"
        "message     TEXT    NOT NULL"
        ")"
        );

    // 索引
    query.exec(
        "CREATE INDEX IF NOT EXISTS idx_timestamp "
        "ON device_data(timestamp)"
        );

    return true;
}

void DatabaseManager::saveData(const DeviceData& data)
{
    if (!data.isValid || !m_db.isOpen()) return;

    m_buffer.append(data);

    if (m_buffer.size() >= BUFFER_SIZE) {
        QSqlQuery query(m_db);
        m_db.transaction();
        for (const auto& d : m_buffer) {
            query.prepare(
                "INSERT INTO device_data "
                "(timestamp, temperature, pressure, status_code) "
                "VALUES (?, ?, ?, ?)"
                );
            query.addBindValue(d.timestamp.toMSecsSinceEpoch());
            query.addBindValue(d.temperature);
            query.addBindValue(d.pressure);
            query.addBindValue(d.statusCode);
            query.exec();
        }
        m_db.commit();
        m_buffer.clear();
    }
}

void DatabaseManager::saveAlarm(const AlarmEvent& event)
{
    if (!m_db.isOpen()) return;

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO alarm_log "
        "(timestamp, channel, alarm_type, value, limit_value, message) "
        "VALUES (?, ?, ?, ?, ?, ?)"
        );
    query.addBindValue(event.timestamp.toMSecsSinceEpoch());
    query.addBindValue(event.channel);
    query.addBindValue(event.type == AlarmEvent::Type::HighLimit
                           ? "HIGH" : "LOW");
    query.addBindValue(event.value);
    query.addBindValue(event.limit);
    query.addBindValue(event.message);
    query.exec();
}

QList<DeviceData> DatabaseManager::queryByTimeRange(
    const QDateTime& from, const QDateTime& to)
{
    QList<DeviceData> result;
    if (!m_db.isOpen()) return result;

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT timestamp, temperature, pressure, status_code "
        "FROM device_data "
        "WHERE timestamp BETWEEN ? AND ? "
        "ORDER BY timestamp ASC"
        );
    query.addBindValue(from.toMSecsSinceEpoch());
    query.addBindValue(to.toMSecsSinceEpoch());
    query.exec();

    while (query.next()) {
        DeviceData data;
        data.timestamp   = QDateTime::fromMSecsSinceEpoch(query.value(0).toLongLong());
        data.temperature = query.value(1).toDouble();
        data.pressure    = query.value(2).toDouble();
        data.statusCode  = query.value(3).toString();
        data.isValid     = true;
        result.append(data);
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
    out << "\xEF\xBB\xBF";  // UTF-8 BOM，让 Excel 正确识别
    out << "时间,温度(℃),压力(MPa),状态\n";
    for (const auto& data : dataList) {
        out << data.timestamp.toString("yyyy-MM-dd hh:mm:ss") << ","
            << QString::number(data.temperature, 'f', 1) << ","
            << QString::number(data.pressure, 'f', 2) << ","
            << data.statusCode << "\n";
    }
    return true;
}
