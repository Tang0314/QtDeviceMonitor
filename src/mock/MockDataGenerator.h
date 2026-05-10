#pragma once

#include <QObject>
#include <QTimer>
#include "data/DeviceData.h"

class MockDataGenerator : public QObject {
    Q_OBJECT

public:
    explicit MockDataGenerator(QObject* parent = nullptr);
    ~MockDataGenerator();

    void start(int intervalMs = 100);  // 开始生成，默认100ms一次
    void stop();
    bool isRunning() const;

signals:
    void dataGenerated(const DeviceData& data);  // 每次生成数据发出此信号

private slots:
    void onTimer();  // 定时器触发，生成一帧数据

private:
    QTimer* m_timer;
    double  m_time;  // 用于正弦波计算的时间变量
};
