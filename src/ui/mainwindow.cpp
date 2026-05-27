#include "ui/mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFont>
#include <QCoreApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_mockGenerator(new MockDataGenerator(this))
    , m_alarmChecker(new AlarmChecker(this))
    , m_dbManager(new DatabaseManager(this))
    , m_virtualDevice(new VirtualTcpDevice(this))
    , m_tcpComm(new TcpComm(this))
    , m_serialComm(new SerialComm(this))
{
    // 从配置文件读取报警阈值
    m_tempConfig  = m_configManager.loadTempConfig();
    m_humConfig   = m_configManager.loadHumConfig();
    m_pressConfig = m_configManager.loadPressConfig();
    m_co2Config   = m_configManager.loadCo2Config();

    m_alarmChecker->setTempConfig(m_tempConfig);
    m_alarmChecker->setHumConfig(m_humConfig);
    m_alarmChecker->setPressConfig(m_pressConfig);
    m_alarmChecker->setCo2Config(m_co2Config);

    setupUI();

    connect(m_mockGenerator, &MockDataGenerator::dataGenerated,
            this, &MainWindow::onDataGenerated);
    connect(m_mockGenerator, &MockDataGenerator::dataGenerated,
            m_alarmChecker, &AlarmChecker::checkData);
    connect(m_alarmChecker, &AlarmChecker::alarmTriggered,
            this, &MainWindow::onAlarmTriggered);
    connect(m_alarmChecker, &AlarmChecker::alarmCleared,
            this, &MainWindow::onAlarmCleared);

    // TCP 通信 — 通过状态机切换
    connect(m_tcpConnBtn, &QPushButton::clicked, this, [this]() {
        if (m_dataSource == DataSource::TCP) {
            setDataSource(DataSource::None);
        } else {
            setDataSource(DataSource::TCP);
        }
    });

    connect(m_tcpComm, &TcpComm::dataReceived,
            this, &MainWindow::onDataGenerated);
    connect(m_tcpComm, &TcpComm::dataReceived,
            m_alarmChecker, &AlarmChecker::checkData);
    connect(m_tcpComm, &TcpComm::dataReceived,
            m_dbManager, &DatabaseManager::saveData);
    connect(m_tcpComm, &TcpComm::connectionStateChanged,
            this, &MainWindow::onTcpStateChanged);
    connect(m_tcpComm, &TcpComm::errorOccurred,
            this, &MainWindow::onTcpError);

    // 初始化数据库（存在程序目录下的 userdata 文件夹）
    m_dbManager->initialize(
        QCoreApplication::applicationDirPath() + "/userdata/QtDeviceMonitor.db"
        );

    // 连接数据存储
    connect(m_mockGenerator, &MockDataGenerator::dataGenerated,
            m_dbManager, &DatabaseManager::saveData);
    connect(m_alarmChecker, &AlarmChecker::alarmTriggered,
            m_dbManager, &DatabaseManager::saveAlarm);

    // 串口通信
    connect(m_serialConnBtn, &QPushButton::clicked,
            this, &MainWindow::onSerialConnect);
    connect(m_serialComm, &SerialComm::dataReceived,
            this, &MainWindow::onDataGenerated);
    connect(m_serialComm, &SerialComm::dataReceived,
            m_alarmChecker, &AlarmChecker::checkData);
    connect(m_serialComm, &SerialComm::dataReceived,
            m_dbManager, &DatabaseManager::saveData);
    connect(m_serialComm, &SerialComm::connectionStateChanged,
            this, &MainWindow::onSerialStateChanged);
}
MainWindow::~MainWindow() {}

// ── 数据源状态机 ──
// 核心原则：先置 m_dataSource = None（中间态）再停止旧数据源，
// 这样停止过程中触发的信号处理程序检测到 m_dataSource 已变化，不会误改 UI。

void MainWindow::setDataSource(DataSource source)
{
    if (m_dataSource == source) return;

    // 1. 先进入 None 中间态，抑制 stop 过程中的信号副作用
    DataSource oldSource = m_dataSource;
    m_dataSource = DataSource::None;

    // 2. 停止旧数据源（此时 m_dataSource == None，信号处理程序会提前 return）
    switch (oldSource) {
    case DataSource::Mock:
        m_mockGenerator->stop();
        break;
    case DataSource::TCP:
        m_tcpComm->disconnectFromDevice();
        m_virtualDevice->stop();
        break;
    case DataSource::Serial:
        m_serialComm->close();
        break;
    case DataSource::None:
        break;
    }

    // 3. 切换到新数据源
    m_dataSource = source;

    // 4. 启动新数据源 + 统一更新全部 UI 控件
    switch (source) {
    case DataSource::None:
        m_startStopBtn->setText("▶ 开始采集");
        m_tcpConnBtn->setText("连接虚拟设备(TCP)");
        m_serialConnBtn->setText("连接串口");
        m_connLabel->setText("● 未连接");
        m_connLabel->setStyleSheet("color: gray;");
        break;

    case DataSource::Mock:
        m_mockGenerator->start(100);
        m_startStopBtn->setText("⏹ 停止采集");
        m_tcpConnBtn->setText("连接虚拟设备(TCP)");
        m_serialConnBtn->setText("连接串口");
        m_connLabel->setText("● Mock 模式");
        m_connLabel->setStyleSheet("color: blue;");
        break;

    case DataSource::TCP:
        m_virtualDevice->start(8888);
        m_tcpComm->connectToDevice("127.0.0.1", 8888);
        m_startStopBtn->setText("▶ 开始采集");
        m_tcpConnBtn->setText("断开TCP");
        m_serialConnBtn->setText("连接串口");
        m_connLabel->setText("● TCP 连接中...");
        m_connLabel->setStyleSheet("color: orange;");
        // 连接成功后 onTcpStateChanged(true) 会更新 connLabel 为绿色
        break;

    case DataSource::Serial:
        // 注意：串口在 onSerialConnect() 中已经 open 成功，
        // onSerialStateChanged(true) 已将 connLabel 设为"● 已连接串口"（绿色），
        // 这里不再覆盖 connLabel
        m_startStopBtn->setText("▶ 开始采集");
        m_tcpConnBtn->setText("连接虚拟设备(TCP)");
        m_serialConnBtn->setText("断开串口");
        break;
    }
}

// ── UI 构建 ──

void MainWindow::setupUI()
{
    setWindowTitle("QtDeviceMonitor - 工业设备监控");
    // 菜单栏
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // 文件菜单
    QMenu* fileMenu = menuBar->addMenu("文件(&F)");

    QAction* exportAction = new QAction("导出 CSV(&E)", this);
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    fileMenu->addAction(exportAction);

    QAction* historyAction = new QAction("历史查询(&H)", this);
    historyAction->setShortcut(QKeySequence("Ctrl+H"));
    fileMenu->addAction(historyAction);
    connect(historyAction, &QAction::triggered, this, &MainWindow::onHistory);

    fileMenu->addSeparator();

    QAction* exitAction = new QAction("退出(&Q)", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    fileMenu->addAction(exitAction);

    // 操作菜单
    QMenu* settingsMenu = menuBar->addMenu("设置(&S)");
    QAction* settingsAction = new QAction("报警阈值(&A)", this);
    settingsMenu->addAction(settingsAction);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);
    QMenu* ctrlMenu = menuBar->addMenu("操作(&C)");
    QAction* startAction = new QAction("开始采集(&S)", this);
    QAction* stopAction  = new QAction("停止采集(&T)", this);
    ctrlMenu->addAction(startAction);
    ctrlMenu->addAction(stopAction);

    // 连接菜单信号
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportCsv);
    connect(exitAction,   &QAction::triggered, this, &QApplication::quit);
    connect(startAction,  &QAction::triggered, [this]{ setDataSource(DataSource::Mock); });
    connect(stopAction,   &QAction::triggered, [this]{ setDataSource(DataSource::None); });
    setMinimumSize(400, 300);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // 数据显示区
    QFont dataFont("Arial", 14, QFont::Bold);
    QFont normalFont("Arial", 11);

    m_tempLabel   = new QLabel("🌡 温度: -- ℃",    this);
    m_humLabel    = new QLabel("💧 湿度: -- %",     this);
    m_pressLabel  = new QLabel("🔵 压力: -- MPa",   this);
    m_co2Label    = new QLabel("🌿 CO₂: -- ppm",   this);
    m_doorLabel   = new QLabel("🚪 门状态: --",     this);
    m_statusLabel = new QLabel("状态: --",          this);
    m_timeLabel   = new QLabel("时间: --",          this);

    m_tempLabel->setFont(dataFont);
    m_humLabel->setFont(dataFont);
    m_pressLabel->setFont(dataFont);
    m_co2Label->setFont(dataFont);
    m_doorLabel->setFont(dataFont);
    m_statusLabel->setFont(dataFont);
    m_timeLabel->setFont(normalFont);

    // 两列布局显示数据
    QGridLayout* dataGrid = new QGridLayout();
    dataGrid->addWidget(m_tempLabel,   0, 0);
    dataGrid->addWidget(m_humLabel,    0, 1);
    dataGrid->addWidget(m_pressLabel,  1, 0);
    dataGrid->addWidget(m_co2Label,    1, 1);
    dataGrid->addWidget(m_doorLabel,   2, 0);
    dataGrid->addWidget(m_statusLabel, 2, 1);
    dataGrid->addWidget(m_timeLabel,   3, 0, 1, 2);
    mainLayout->addLayout(dataGrid);

    // TCP 连接控制
    m_connLabel  = new QLabel("● 未连接", this);
    m_connLabel->setStyleSheet("color: gray;");
    m_tcpConnBtn = new QPushButton("连接虚拟设备(TCP)", this);
    m_tcpConnBtn->setFixedHeight(35);

    QHBoxLayout* connLayout = new QHBoxLayout();
    connLayout->addWidget(m_connLabel);
    connLayout->addStretch();
    connLayout->addWidget(m_tcpConnBtn);
    mainLayout->addLayout(connLayout);

    // 串口连接控制
    m_serialConnBtn = new QPushButton("连接串口", this);
    m_serialConnBtn->setFixedHeight(35);
    connLayout->addWidget(m_serialConnBtn);

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
        QString("🌡 温度: %1 ℃").arg(data.temperature, 0, 'f', 1));
    m_humLabel->setText(
        QString("💧 湿度: %1 %").arg(data.humidity, 0, 'f', 1));
    m_pressLabel->setText(
        QString("🔵 压力: %1 MPa").arg(data.pressure, 0, 'f', 4));
    m_co2Label->setText(
        QString("🌿 CO₂: %1 ppm").arg(data.co2, 0, 'f', 0));

    // 门状态
    if (data.doorOpen) {
        m_doorLabel->setText("🚪 门状态: ⚠ 开启");
        m_doorLabel->setStyleSheet("color: orange;");
    } else {
        m_doorLabel->setText("🚪 门状态: ✓ 关闭");
        m_doorLabel->setStyleSheet("color: green;");
    }

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
    if (m_dataSource == DataSource::Mock) {
        setDataSource(DataSource::None);
    } else {
        setDataSource(DataSource::Mock);
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

void MainWindow::onExportCsv()
{
    QString filePath = QFileDialog::getSaveFileName(
        this, "导出 CSV",
        QCoreApplication::applicationDirPath() + "/userdata/export.csv",
        "CSV 文件 (*.csv)"
        );
    if (filePath.isEmpty()) return;

    QDateTime to   = QDateTime::currentDateTime();
    QDateTime from = to.addSecs(-3600);  // 导出最近1小时

    if (m_dbManager->exportToCsv(filePath, from, to)) {
        QMessageBox::information(this, "导出成功",
                                 "数据已导出到：\n" + filePath);
    } else {
        QMessageBox::warning(this, "导出失败", "没有数据或文件写入失败");
    }
}

void MainWindow::onSettings()
{
    SettingsDialog dlg(this);
    dlg.setTempConfig(m_tempConfig);
    dlg.setHumConfig(m_humConfig);
    dlg.setPressConfig(m_pressConfig);
    dlg.setCo2Config(m_co2Config);

    if (dlg.exec() == QDialog::Accepted) {
        m_tempConfig  = dlg.getTempConfig();
        m_humConfig   = dlg.getHumConfig();
        m_pressConfig = dlg.getPressConfig();
        m_co2Config   = dlg.getCo2Config();

        m_alarmChecker->setTempConfig(m_tempConfig);
        m_alarmChecker->setHumConfig(m_humConfig);
        m_alarmChecker->setPressConfig(m_pressConfig);
        m_alarmChecker->setCo2Config(m_co2Config);

        // 保存到配置文件
        m_configManager.saveTempConfig(m_tempConfig);
        m_configManager.saveHumConfig(m_humConfig);
        m_configManager.savePressConfig(m_pressConfig);
        m_configManager.saveCo2Config(m_co2Config);
    }
}

void MainWindow::onHistory()
{
    HistoryDialog dlg(m_dbManager, this);
    dlg.exec();
}

void MainWindow::onTcpStateChanged(bool connected)
{
    if (m_dataSource != DataSource::TCP) return; // 已切换到其他数据源，忽略

    if (connected) {
        m_connLabel->setText("● 已连接 TCP 127.0.0.1:8888");
        m_connLabel->setStyleSheet("color: green;");
    } else {
        // TCP 意外断开：自动重连中，更新标签提示
        m_connLabel->setText("● TCP 断开，重连中...");
        m_connLabel->setStyleSheet("color: orange;");
    }
}

void MainWindow::onTcpError(const QString& msg)
{
    m_connLabel->setText("● 错误: " + msg);
    m_connLabel->setStyleSheet("color: red;");
}

void MainWindow::onSerialConnect()
{
    if (m_dataSource == DataSource::Serial) {
        setDataSource(DataSource::None);
        return;
    }

    SerialConfigDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        SerialConfig config = dlg.getConfig();
        if (m_serialComm->open(config)) {
            setDataSource(DataSource::Serial);
        }
    }
}

void MainWindow::onSerialStateChanged(bool connected)
{
    if (connected) {
        m_connLabel->setText("● 已连接串口");
        m_connLabel->setStyleSheet("color: green;");
    } else {
        // 只有当前是串口模式才更新UI（避免切换数据源时的闪烁）
        if (m_dataSource == DataSource::Serial) {
            m_serialConnBtn->setText("连接串口");
            m_connLabel->setText("● 串口已断开");
            m_connLabel->setStyleSheet("color: gray;");
        }
    }
}
