#pragma once

#include <QString>
#include <QDateTime>

// 设备数据结构体 - 所有模块共用
struct DeviceData {
    QDateTime timestamp;     // 采集时间
    double    temperature;   // 温度 (℃)
    double    pressure;      // 压力 (MPa)
    QString   statusCode;    // 状态码: OK / WARN / ALARM / ERROR
    bool      isValid;       // 数据是否有效

    DeviceData()
        : temperature(0.0)
        , pressure(0.0)
        , statusCode("OK")
        , isValid(false)
    {}
};
