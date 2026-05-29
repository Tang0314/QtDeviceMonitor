#pragma once

#include <QObject>
#include <QString>
#include "data/DeviceData.h"

// 各通道默认报警阈值 —— 唯一定义点
namespace AlarmDefaults {
    constexpr double TEMP_HIGH  = -15.0;
    constexpr double TEMP_LOW   = -23.0;
    constexpr double HUM_HIGH   = 95.0;
    constexpr double HUM_LOW    = 60.0;
    constexpr double PRESS_HIGH = 0.1060;
    constexpr double PRESS_LOW  = 0.0966;
    constexpr double CO2_HIGH   = 1000.0;
    constexpr double CO2_LOW    = 0.0;
}

struct AlarmConfig {
    double  highLimit = 30.0;
    double  lowLimit  = 10.0;
    bool    enabled   = true;
};

struct AlarmEvent {
    QDateTime timestamp;
    QString   channel;
    double    value;
    double    limit;
    enum class Type { HighLimit, LowLimit } type;
    QString   message;
};

class AlarmChecker : public QObject {
    Q_OBJECT

public:
    explicit AlarmChecker(QObject* parent = nullptr);

    void setTempConfig(const AlarmConfig& config);
    void setHumConfig(const AlarmConfig& config);
    void setPressConfig(const AlarmConfig& config);
    void setCo2Config(const AlarmConfig& config);

public slots:
    void checkData(const DeviceData& data);

signals:
    void alarmTriggered(const AlarmEvent& event);
    void alarmCleared(const QString& channel);

private:
    void checkChannel(const QString& channel, double value,
                      const AlarmConfig& config, bool& alarmFlag);

    AlarmConfig m_tempConfig;
    AlarmConfig m_humConfig;
    AlarmConfig m_pressConfig;
    AlarmConfig m_co2Config;

    bool m_tempAlarming  = false;
    bool m_humAlarming   = false;
    bool m_pressAlarming = false;
    bool m_co2Alarming   = false;
    bool m_doorAlarming  = false;
};
