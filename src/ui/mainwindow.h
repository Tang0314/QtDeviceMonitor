#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "mock/MockDataGenerator.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onDataGenerated(const DeviceData& data);
    void onStartStopClicked();

private:
    void setupUI();

    // UI 控件
    QLabel*      m_tempLabel;      // 温度显示
    QLabel*      m_pressLabel;     // 压力显示
    QLabel*      m_statusLabel;    // 状态码显示
    QLabel*      m_timeLabel;      // 时间显示
    QPushButton* m_startStopBtn;   // 启动/停止按钮

    // 数据生成器
    MockDataGenerator* m_mockGenerator;
};
