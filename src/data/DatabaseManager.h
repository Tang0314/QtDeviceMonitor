#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "data/DeviceData.h"
#include "alarm/AlarmChecker.h"

class DatabaseManager : public QObject {
    Q_OBJECT

public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    bool initialize(const QString& dbPath);  // 初始化数据库

public slots:
    void saveData(const DeviceData& data);           // 存储采集数据
    void saveAlarm(const AlarmEvent& event);         // 存储报警记录

public:
    QList<DeviceData> queryByTimeRange(
        const QDateTime& from,
        const QDateTime& to
        );
    bool exportToCsv(
        const QString& filePath,
        const QDateTime& from,
        const QDateTime& to
        );

private:
    bool createTables();

    QSqlDatabase m_db;
    QList<DeviceData> m_buffer;          // 写入缓冲区
    static const int BUFFER_SIZE = 10;   // 每10条批量写入
};
