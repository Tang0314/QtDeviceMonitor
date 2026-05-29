#pragma once

#include "DeviceData.h"
#include <QByteArray>
#include <QDateTime>
#include <QList>

// 数据帧解析工具 —— TcpComm 和 SerialComm 共用
class DataParser {
public:
    // 解析 CSV 帧：温度,湿度,压力,CO2,门状态,状态码
    // 解析失败时返回的 DeviceData.isValid == false
    static DeviceData parse(const QByteArray& frame);
};
