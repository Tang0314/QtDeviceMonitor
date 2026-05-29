#pragma once

#include <QtMath>
#include <QDateTime>
#include <QString>
#include "data/DeviceData.h"

// Mock 数据生成公式 —— MockDataGenerator 和 VirtualTcpDevice 共用
namespace MockDataFormulas {

// 根据时间变量生成一帧 DeviceData
inline DeviceData generate(double time)
{
    DeviceData data;
    data.timestamp   = QDateTime::currentDateTime();
    data.temperature = -18.0 + 3.0  * qSin(time * 0.05);
    data.humidity    = 85.0  + 10.0 * qSin(time * 0.08 + 1.0);
    data.pressure    = 0.1013 + 0.005 * qSin(time * 0.03 + 2.0);
    data.co2         = 800.0 + 300.0 * qSin(time * 0.06 + 0.5);

    int cycle  = static_cast<int>(time) % 300;
    data.doorOpen = (cycle >= 200 && cycle < 230);

    // 综合状态判断
    bool tempAlarm = data.temperature > -15.0 || data.temperature < -23.0;
    bool humAlarm  = data.humidity > 95.0 || data.humidity < 60.0;
    bool co2Alarm  = data.co2 > 1000.0;
    bool doorAlarm = data.doorOpen;

    if (tempAlarm || co2Alarm)
        data.statusCode = "ALARM";
    else if (humAlarm || doorAlarm)
        data.statusCode = "WARN";
    else
        data.statusCode = "OK";

    data.isValid = true;
    return data;
}

// 生成 CSV 帧字符串（用于 VirtualTcpDevice 发送）
// 提高精度以减少 CSV 往返过程的精度损失
inline QString toCsvFrame(const DeviceData& data)
{
    return QString("%1,%2,%3,%4,%5,%6")
        .arg(data.temperature, 0, 'f', 2)
        .arg(data.humidity,    0, 'f', 2)
        .arg(data.pressure,    0, 'f', 6)
        .arg(data.co2,         0, 'f', 1)
        .arg(data.doorOpen ? 1 : 0)
        .arg(data.statusCode);
}

} // namespace MockDataFormulas
