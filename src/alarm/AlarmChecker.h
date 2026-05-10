#pragma once

#include <QObject>
#include <QString>
#include "data/DeviceData.h"

// 单个通道的报警配置
struct AlarmConfig {
    double  highLimit   = 30.0;   // 上限
    double  lowLimit    = 10.0;   // 下限
    bool    enabled     = true;   // 是否启用
};

// 报警事件
struct AlarmEvent {
    QDateTime timestamp;
    QString   channel;      // "temperature" / "pressure"
    double    value;        // 触发时的值
    double    limit;        // 触发的阈值
    enum class Type { HighLimit, LowLimit } type;
    QString   message;      // 可读描述
};

class AlarmChecker : public QObject {
    Q_OBJECT

public:
    explicit AlarmChecker(QObject* parent = nullptr);

    void setTempConfig(const AlarmConfig& config);
    void setPressConfig(const AlarmConfig& config);

public slots:
    void checkData(const DeviceData& data);  // 检测数据是否超限

signals:
    void alarmTriggered(const AlarmEvent& event);  // 触发报警
    void alarmCleared(const QString& channel);     // 报警解除

private:
    AlarmConfig m_tempConfig;
    AlarmConfig m_pressConfig;

    bool m_tempAlarming  = false;  // 温度是否正在报警
    bool m_pressAlarming = false;  // 压力是否正在报警
};
