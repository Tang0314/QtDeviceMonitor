#pragma once

#include "DeviceData.h"
#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QString>

struct ParseResult {
    DeviceData data;
    QString error;

    bool isValid() const { return data.isValid; }
};

// 数据帧解析工具 —— TcpComm 和 SerialComm 共用
class DataParser {
public:
    // 解析 CSV 帧：温度,湿度,压力,CO2,门状态,状态码
    // 解析失败时返回的 DeviceData.isValid == false
    static DeviceData parse(const QByteArray& frame);
    static ParseResult parseFrame(const QByteArray& frame);
};
