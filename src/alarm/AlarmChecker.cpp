#include "alarm/AlarmChecker.h"

AlarmChecker::AlarmChecker(QObject* parent)
    : QObject(parent)
{}

void AlarmChecker::setTempConfig(const AlarmConfig& config)
{
    m_tempConfig   = config;
    m_tempAlarming = false;  // 重置状态
}

void AlarmChecker::setPressConfig(const AlarmConfig& config)
{
    m_pressConfig   = config;
    m_pressAlarming = false;  // 重置状态
}

void AlarmChecker::checkData(const DeviceData& data)
{
    if (!data.isValid) return;

    // ── 温度检测 ──
    if (m_tempConfig.enabled) {
        bool over = data.temperature > m_tempConfig.highLimit
                    || data.temperature < m_tempConfig.lowLimit;

        if (over && !m_tempAlarming) {
            m_tempAlarming = true;
            AlarmEvent evt;
            evt.timestamp = data.timestamp;
            evt.channel   = "temperature";
            evt.value     = data.temperature;
            if (data.temperature > m_tempConfig.highLimit) {
                evt.type    = AlarmEvent::Type::HighLimit;
                evt.limit   = m_tempConfig.highLimit;
                evt.message = QString("温度超上限: %1℃ > %2℃")
                                  .arg(data.temperature, 0, 'f', 1)
                                  .arg(m_tempConfig.highLimit, 0, 'f', 1);
            } else {
                evt.type    = AlarmEvent::Type::LowLimit;
                evt.limit   = m_tempConfig.lowLimit;
                evt.message = QString("温度低于下限: %1℃ < %2℃")
                                  .arg(data.temperature, 0, 'f', 1)
                                  .arg(m_tempConfig.lowLimit, 0, 'f', 1);
            }
            emit alarmTriggered(evt);
        } else if (!over && m_tempAlarming) {
            m_tempAlarming = false;
            emit alarmCleared("temperature");
        }
    }

    // ── 压力检测 ──
    if (m_pressConfig.enabled) {
        bool over = data.pressure > m_pressConfig.highLimit
                    || data.pressure < m_pressConfig.lowLimit;

        if (over && !m_pressAlarming) {
            m_pressAlarming = true;
            AlarmEvent evt;
            evt.timestamp = data.timestamp;
            evt.channel   = "pressure";
            evt.value     = data.pressure;
            if (data.pressure > m_pressConfig.highLimit) {
                evt.type    = AlarmEvent::Type::HighLimit;
                evt.limit   = m_pressConfig.highLimit;
                evt.message = QString("压力超上限: %1MPa > %2MPa")
                                  .arg(data.pressure, 0, 'f', 2)
                                  .arg(m_pressConfig.highLimit, 0, 'f', 2);
            } else {
                evt.type    = AlarmEvent::Type::LowLimit;
                evt.limit   = m_pressConfig.lowLimit;
                evt.message = QString("压力低于下限: %1MPa < %2MPa")
                                  .arg(data.pressure, 0, 'f', 2)
                                  .arg(m_pressConfig.lowLimit, 0, 'f', 2);
            }
            emit alarmTriggered(evt);
        } else if (!over && m_pressAlarming) {
            m_pressAlarming = false;
            emit alarmCleared("pressure");
        }
    }
}
