#include "alarm/AlarmChecker.h"

#include <QDebug>

AlarmConfig normalizedAlarmConfig(const AlarmConfig& config,
                                  double defaultHigh,
                                  double defaultLow)
{
    AlarmConfig normalized = config;
    if (normalized.highLimit <= normalized.lowLimit) {
        qWarning() << "Invalid alarm config, fallback to defaults:"
                   << normalized.lowLimit << normalized.highLimit;
        normalized.highLimit = defaultHigh;
        normalized.lowLimit = defaultLow;
    }
    return normalized;
}

AlarmChecker::AlarmChecker(QObject* parent)
    : QObject(parent)
{
    m_tempConfig.highLimit  = AlarmDefaults::TEMP_HIGH;
    m_tempConfig.lowLimit   = AlarmDefaults::TEMP_LOW;

    m_humConfig.highLimit   = AlarmDefaults::HUM_HIGH;
    m_humConfig.lowLimit    = AlarmDefaults::HUM_LOW;

    m_pressConfig.highLimit = AlarmDefaults::PRESS_HIGH;
    m_pressConfig.lowLimit  = AlarmDefaults::PRESS_LOW;

    m_co2Config.highLimit   = AlarmDefaults::CO2_HIGH;
    m_co2Config.lowLimit    = AlarmDefaults::CO2_LOW;
}

void AlarmChecker::setTempConfig(const AlarmConfig& config)
{
    m_tempConfig   = normalizedAlarmConfig(config,
                                           AlarmDefaults::TEMP_HIGH,
                                           AlarmDefaults::TEMP_LOW);
    m_tempAlarming = false;
}

void AlarmChecker::setHumConfig(const AlarmConfig& config)
{
    m_humConfig   = normalizedAlarmConfig(config,
                                          AlarmDefaults::HUM_HIGH,
                                          AlarmDefaults::HUM_LOW);
    m_humAlarming = false;
}

void AlarmChecker::setPressConfig(const AlarmConfig& config)
{
    m_pressConfig   = normalizedAlarmConfig(config,
                                            AlarmDefaults::PRESS_HIGH,
                                            AlarmDefaults::PRESS_LOW);
    m_pressAlarming = false;
}

void AlarmChecker::setCo2Config(const AlarmConfig& config)
{
    m_co2Config   = normalizedAlarmConfig(config,
                                          AlarmDefaults::CO2_HIGH,
                                          AlarmDefaults::CO2_LOW);
    m_co2Alarming = false;
}

void AlarmChecker::checkChannel(
    const QString& channel, double value,
    const AlarmConfig& config, bool& alarmFlag)
{
    if (!config.enabled) return;
    if (config.highLimit <= config.lowLimit) return;

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
