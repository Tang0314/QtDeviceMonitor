#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QTimer>
#include "data/DeviceData.h"
#include "alarm/AlarmChecker.h"

// 数据库写入工作线程 —— 独立 SQLite 连接，避免主线程 I/O 阻塞
class DatabaseWorker : public QObject {
    Q_OBJECT

public:
    explicit DatabaseWorker(QObject* parent = nullptr);
    ~DatabaseWorker();

public slots:
    bool initialize(const QString& dbPath, const QString& connectionName);
    void saveData(const DeviceData& data);
    void saveAlarm(const AlarmEvent& event);
    void flush();

private:
    bool createTables();
    bool flushData();
    bool flushAlarms();

    QSqlDatabase m_db;
    QString m_dbPath;
    QString m_connectionName;
    QTimer* m_flushTimer = nullptr;

    QList<DeviceData> m_dataBuffer;
    QList<AlarmEvent> m_alarmBuffer;
    static const int BATCH_SIZE = 50;
    static const int ALARM_BATCH_SIZE = 10;
};
