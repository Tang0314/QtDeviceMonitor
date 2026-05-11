#include "mock/MockDataGenerator.h"
#include <QtMath>
#include <QRandomGenerator>

MockDataGenerator::MockDataGenerator(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_time(0.0)
{
    connect(m_timer, &QTimer::timeout, this, &MockDataGenerator::onTimer);
}

MockDataGenerator::~MockDataGenerator()
{
    stop();
}

void MockDataGenerator::start(int intervalMs)
{
    m_time = 0.0;
    m_timer->start(intervalMs);
}

void MockDataGenerator::stop()
{
    m_timer->stop();
}

bool MockDataGenerator::isRunning() const
{
    return m_timer->isActive();
}

void MockDataGenerator::onTimer()
{
    DeviceData data;
    data.timestamp = QDateTime::currentDateTime();

    // 冷库温度：-18℃ ± 3℃ 正弦波
    data.temperature = -18.0 + 3.0 * qSin(m_time * 0.05);

    // 湿度：85% ± 10%
    data.humidity = 85.0 + 10.0 * qSin(m_time * 0.08 + 1.0);

    // 压力：标准大气压附近 0.1013 ± 0.005 MPa
    data.pressure = 0.1013 + 0.005 * qSin(m_time * 0.03 + 2.0);

    // CO₂浓度：800ppm ± 300ppm
    data.co2 = 800.0 + 300.0 * qSin(m_time * 0.06 + 0.5);

    // 门状态：每300个周期模拟开门一次，持续30个周期
    int cycle = (int)m_time % 300;
    data.doorOpen = (cycle >= 200 && cycle < 230);

    // 综合状态判断
    bool tempAlarm  = data.temperature > -15.0 || data.temperature < -23.0;
    bool humAlarm   = data.humidity > 95.0 || data.humidity < 60.0;
    bool co2Alarm   = data.co2 > 1000.0;
    bool doorAlarm  = data.doorOpen;

    if (tempAlarm || co2Alarm) {
        data.statusCode = "ALARM";
    } else if (humAlarm || doorAlarm) {
        data.statusCode = "WARN";
    } else {
        data.statusCode = "OK";
    }

    data.isValid = true;
    m_time += 1.0;

    emit dataGenerated(data);
}
