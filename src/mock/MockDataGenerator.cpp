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

    // 正弦波模拟温度：25℃ ± 5℃
    data.temperature = 25.0 + 5.0 * qSin(m_time * 0.1);

    // 正弦波模拟压力：1.05 MPa ± 0.1
    data.pressure = 1.05 + 0.1 * qSin(m_time * 0.15);

    // 状态码：温度超过28℃触发ALARM
    if (data.temperature > 28.0) {
        data.statusCode = "ALARM";
    } else if (data.temperature > 27.0) {
        data.statusCode = "WARN";
    } else {
        data.statusCode = "OK";
    }

    data.isValid = true;
    m_time += 1.0;

    emit dataGenerated(data);
}
