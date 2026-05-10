#include "ui/mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFont>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_mockGenerator(new MockDataGenerator(this))
    , m_alarmChecker(new AlarmChecker(this))
{
    // 配置报警阈值
    AlarmConfig tempConfig;
    tempConfig.highLimit = 28.0;
    tempConfig.lowLimit  = 15.0;
    m_alarmChecker->setTempConfig(tempConfig);

    AlarmConfig pressConfig;
    pressConfig.highLimit = 1.2;
    pressConfig.lowLimit  = 0.9;
    m_alarmChecker->setPressConfig(pressConfig);

    setupUI();

    connect(m_mockGenerator, &MockDataGenerator::dataGenerated,
            this, &MainWindow::onDataGenerated);
    connect(m_mockGenerator, &MockDataGenerator::dataGenerated,
            m_alarmChecker, &AlarmChecker::checkData);
    connect(m_alarmChecker, &AlarmChecker::alarmTriggered,
            this, &MainWindow::onAlarmTriggered);
    connect(m_alarmChecker, &AlarmChecker::alarmCleared,
            this, &MainWindow::onAlarmCleared);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    setWindowTitle("QtDeviceMonitor - 工业设备监控");
    setMinimumSize(400, 300);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // 数据显示区
    QFont dataFont("Arial", 16, QFont::Bold);

    m_tempLabel   = new QLabel("温度: -- ℃", this);
    m_pressLabel  = new QLabel("压力: -- MPa", this);
    m_statusLabel = new QLabel("状态: --", this);
    m_timeLabel   = new QLabel("时间: --", this);

    m_tempLabel->setFont(dataFont);
    m_pressLabel->setFont(dataFont);
    m_statusLabel->setFont(dataFont);

    mainLayout->addWidget(m_tempLabel);
    mainLayout->addWidget(m_pressLabel);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_timeLabel);

    // 启动/停止按钮
    m_startStopBtn = new QPushButton("▶ 开始采集", this);
    m_startStopBtn->setFixedHeight(40);
    connect(m_startStopBtn, &QPushButton::clicked,
            this, &MainWindow::onStartStopClicked);

    // 曲线图
    m_chartWidget = new ChartWidget(this);
    m_chartWidget->setMinimumHeight(300);
    mainLayout->addWidget(m_chartWidget);

    // 报警列表
    QLabel* alarmTitle = new QLabel("📋 报警记录", this);
    m_alarmList = new QListWidget(this);
    m_alarmList->setMaximumHeight(120);
    mainLayout->addWidget(alarmTitle);
    mainLayout->addWidget(m_alarmList);

    mainLayout->addStretch();
    mainLayout->addWidget(m_startStopBtn);
}

void MainWindow::onDataGenerated(const DeviceData& data)
{
    m_tempLabel->setText(
        QString("温度: %1 ℃").arg(data.temperature, 0, 'f', 1));
    m_pressLabel->setText(
        QString("压力: %1 MPa").arg(data.pressure, 0, 'f', 2));
    m_timeLabel->setText(
        QString("时间: %1").arg(data.timestamp.toString("hh:mm:ss")));

    // 状态颜色
    if (data.statusCode == "ALARM") {
        m_statusLabel->setText("状态: ⚠ ALARM");
        m_statusLabel->setStyleSheet("color: red;");
    } else if (data.statusCode == "WARN") {
        m_statusLabel->setText("状态: ⚡ WARN");
        m_statusLabel->setStyleSheet("color: orange;");
    } else {
        m_statusLabel->setText("状态: ✓ OK");
        m_statusLabel->setStyleSheet("color: green;");
    }

    m_chartWidget->addData(data);
}

void MainWindow::onStartStopClicked()
{
    if (m_mockGenerator->isRunning()) {
        m_mockGenerator->stop();
        m_startStopBtn->setText("▶ 开始采集");
    } else {
        m_mockGenerator->start(100);
        m_startStopBtn->setText("⏹ 停止采集");
    }
}

void MainWindow::onAlarmTriggered(const AlarmEvent& event)
{
    QString item = QString("[%1] ⚠ %2")
                       .arg(event.timestamp.toString("hh:mm:ss"))
                       .arg(event.message);
    m_alarmList->insertItem(0, item);
    m_alarmList->item(0)->setForeground(Qt::red);
}

void MainWindow::onAlarmCleared(const QString& channel)
{
    QString item = QString("[%1] ✓ %2 恢复正常")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                       .arg(channel);
    m_alarmList->insertItem(0, item);
    m_alarmList->item(0)->setForeground(Qt::darkGreen);
}
