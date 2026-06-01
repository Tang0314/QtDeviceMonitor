#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "data/DeviceData.h"
#include "alarm/AlarmChecker.h"

class DatabaseWorker;
class QThread;

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    // 阻塞直到 worker 线程初始化完成
    bool initialize(const QString& dbPath);

public slots:
    // 通过 invokeMethod 异步转发到 worker 线程执行写入
    void saveData(const DeviceData& data);
    void saveAlarm(const AlarmEvent& event);
    void flush();

public:
    QList<DeviceData> queryByTimeRange(
        const QDateTime& from,
        const QDateTime& to,
        int offset = -1,
        int limit  = -1
        );
    int countByTimeRange(
        const QDateTime& from,
        const QDateTime& to
        );
    QList<AlarmEvent> queryAlarmsByTimeRange(
        const QDateTime& from,
        const QDateTime& to
        );
    bool exportToCsv(
        const QString& filePath,
        const QDateTime& from,
        const QDateTime& to
        );

private:
    void createTables();   // 主线程连接建表
    bool executeSchema(QSqlDatabase& db);

    QSqlDatabase m_db;     // 主线程连接（只用于查询）
    QString      m_mainConnectionName;
    QString      m_workerConnectionName;
    bool         m_initialized = false;

    QThread*        m_workerThread;
    DatabaseWorker* m_worker;
};
