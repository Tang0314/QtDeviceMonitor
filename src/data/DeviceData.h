#pragma once

#include <QString>
#include <QDateTime>
#include <QMetaType>

namespace DeviceDataLimits {
    constexpr double TEMP_MIN = -30.0;
    constexpr double TEMP_MAX = 10.0;
    constexpr double HUM_MIN = 0.0;
    constexpr double HUM_MAX = 100.0;
    constexpr double PRESS_MIN = 0.08;
    constexpr double PRESS_MAX = 0.12;
    constexpr double CO2_MIN = 0.0;
    constexpr double CO2_MAX = 5000.0;
}

// 设备数据结构体 - 冷链仓储监控
struct DeviceData {
    QDateTime timestamp;      // 采集时间
    double    temperature;    // 温度 (℃)      范围: -30 ~ 10
    double    humidity;       // 湿度 (%)       范围: 0 ~ 100
    double    pressure;       // 压力 (MPa)     范围: 0.08 ~ 0.12
    double    co2;            // CO₂浓度 (ppm)  范围: 0 ~ 5000
    bool      doorOpen;       // 门状态         true=开 false=关
    QString   statusCode;     // OK/WARN/ALARM/ERROR
    bool      isValid;        // 数据是否有效

    DeviceData()
        : temperature(0.0)
        , humidity(0.0)
        , pressure(0.1013)
        , co2(400.0)
        , doorOpen(false)
        , statusCode("OK")
        , isValid(false)
    {}
};

Q_DECLARE_METATYPE(DeviceData)
