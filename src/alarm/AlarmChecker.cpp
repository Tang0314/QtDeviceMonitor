#include "alarm/AlarmChecker.h"

AlarmChecker::AlarmChecker(QObject* parent)
    : QObject(parent)
{
    // 冷链仓储默认阈值
    m_tempConfig.highLimit  = -15.0;
    m_tempConfig.lowLimit   = -23.0;

    m_humConfig.highLimit   = 95.0;
    m_humConfig.lowLimit    = 60.0;

    m_pressConfig.highLimit = 0.1060;
    m_pressConfig.lowLimit  = 0.0966;

    m_co2Config.highLimit   = 1000.0;
    m_co2Config.lowLimit    = 0.0;
}

void AlarmChecker::setTempConfig(const AlarmConfig& config)
{
    m_tempConfig   = config;
    m_tempAlarming = false;
}

void AlarmChecker::setHumConfig(const AlarmConfig& config)
{
    m_humConfig   = config;
    m_humAlarming = false;
}

void AlarmChecker::setPressConfig(const AlarmConfig& config)
{
    m_pressConfig   = config;
    m_pressAlarming = false;
}

void AlarmChecker::setCo2Config(const AlarmConfig& config)
{
    m_co2Config   = config;
    m_co2Alarming = false;
}

void AlarmChecker::checkChannel(
    const QString& channel, double value,
    const AlarmConfig& config, bool& alarmFlag)
{
    if (!config.enabled) return;

    bool over = value > config.highLimit || value < config.lowLimit;

    if (over && !alarmFlag) {
        alarmFlag = true;
        AlarmEvent evt;
        evt.timestamp = QDateTime::currentDateTime();
        evt.channel   = channel;
        evt.value     = value;

        if (value > config.highLimit) {
            evt.type    = AlarmEvent::Type::HighLimit;
            evt.limit   = config.highLimit;
            evt.message = QString("%1 超上限: %2 > %3")
                              .arg(channel)
                              .arg(value, 0, 'f', 2)
                              .arg(config.highLimit, 0, 'f', 2);
        } else {
            evt.type    = AlarmEvent::Type::LowLimit;
            evt.limit   = config.lowLimit;
            evt.message = QString("%1 低于下限: %2 < %3")
                              .arg(channel)
                              .arg(value, 0, 'f', 2)
                              .arg(config.lowLimit, 0, 'f', 2);
        }
        emit alarmTriggered(evt);

    } else if (!over && alarmFlag) {
        alarmFlag = false;
        emit alarmCleared(channel);
    }
}

void AlarmChecker::checkData(const DeviceData& data)
{
    if (!data.isValid) return;

    checkChannel("温度",   data.temperature, m_tempConfig,  m_tempAlarming);
    checkChannel("湿度",   data.humidity,    m_humConfig,   m_humAlarming);
    checkChannel("压力",   data.pressure,    m_pressConfig, m_pressAlarming);
    checkChannel("CO₂",   data.co2,         m_co2Config,   m_co2Alarming);

    // 门状态报警
    if (data.doorOpen && !m_doorAlarming) {
        m_doorAlarming = true;
        AlarmEvent evt;
        evt.timestamp = QDateTime::currentDateTime();
        evt.channel   = "门状态";
        evt.value     = 1;
        evt.limit     = 0;
        evt.type      = AlarmEvent::Type::HighLimit;
        evt.message   = "⚠ 冷库门已开启！";
        emit alarmTriggered(evt);
    } else if (!data.doorOpen && m_doorAlarming) {
        m_doorAlarming = false;
        emit alarmCleared("门状态");
    }
}
