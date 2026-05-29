#include "mock/MockDataGenerator.h"
#include "mock/MockDataFormulas.h"

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
    DeviceData data = MockDataFormulas::generate(m_time);
    m_time += 1.0;
    emit dataGenerated(data);
}
