#include "ui/mainwindow.h"

#include "ui/AlarmHistoryDialog.h"
#include "ui/HistoryDialog.h"
#include "ui/SerialConfigDialog.h"
#include "ui/SettingsDialog.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_controller(new DeviceMonitorController(this))
{
    setupUI();
    connectSignals();
    initializeServices();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    setWindowTitle("QtDeviceMonitor - 工业设备监控");

    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu* fileMenu = menuBar->addMenu("文件(&F)");

    QAction* exportAction = new QAction("导出 CSV(&E)", this);
    exportAction->setShortcut(QKeySequence("Ctrl+E"));
    fileMenu->addAction(exportAction);

    QAction* historyAction = new QAction("历史查询(&H)", this);
    historyAction->setShortcut(QKeySequence("Ctrl+H"));
    fileMenu->addAction(historyAction);

    QAction* alarmHistoryAction = new QAction("报警历史(&L)", this);
    fileMenu->addAction(alarmHistoryAction);

    fileMenu->addSeparator();

    QAction* exitAction = new QAction("退出(&Q)", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    fileMenu->addAction(exitAction);

    QMenu* settingsMenu = menuBar->addMenu("设置(&S)");
    QAction* settingsAction = new QAction("报警阈值(&A)", this);
    settingsMenu->addAction(settingsAction);

    QMenu* ctrlMenu = menuBar->addMenu("操作(&C)");
    QAction* startAction = new QAction("开始采集(&S)", this);
    QAction* stopAction = new QAction("停止采集(&T)", this);
    ctrlMenu->addAction(startAction);
    ctrlMenu->addAction(stopAction);

    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportCsv);
    connect(historyAction, &QAction::triggered, this, &MainWindow::onHistory);
    connect(alarmHistoryAction, &QAction::triggered, this, &MainWindow::onAlarmHistory);
    connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);
    connect(startAction, &QAction::triggered, this, [this] { m_controller->startMock(); });
    connect(stopAction, &QAction::triggered, m_controller, &DeviceMonitorController::stop);

    setMinimumSize(720, 560);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    QFont dataFont("Arial", 14, QFont::Bold);
    QFont normalFont("Arial", 11);

    m_tempLabel = new QLabel("温度: -- ℃", this);
    m_humLabel = new QLabel("湿度: -- %", this);
    m_pressLabel = new QLabel("压力: -- MPa", this);
    m_co2Label = new QLabel("CO₂: -- ppm", this);
    m_doorLabel = new QLabel("门状态: --", this);
    m_statusLabel = new QLabel("状态: --", this);
    m_timeLabel = new QLabel("时间: --", this);

    m_tempLabel->setFont(dataFont);
    m_humLabel->setFont(dataFont);
    m_pressLabel->setFont(dataFont);
    m_co2Label->setFont(dataFont);
    m_doorLabel->setFont(dataFont);
    m_statusLabel->setFont(dataFont);
    m_timeLabel->setFont(normalFont);

    QGridLayout* dataGrid = new QGridLayout();
    dataGrid->addWidget(m_tempLabel, 0, 0);
    dataGrid->addWidget(m_humLabel, 0, 1);
    dataGrid->addWidget(m_pressLabel, 1, 0);
    dataGrid->addWidget(m_co2Label, 1, 1);
    dataGrid->addWidget(m_doorLabel, 2, 0);
    dataGrid->addWidget(m_statusLabel, 2, 1);
    dataGrid->addWidget(m_timeLabel, 3, 0, 1, 2);
    mainLayout->addLayout(dataGrid);

    m_connLabel = new QLabel("● 未连接", this);
    m_connLabel->setStyleSheet("color: gray;");
    m_tcpConnBtn = new QPushButton("连接虚拟设备(TCP)", this);
    m_tcpConnBtn->setFixedHeight(35);
    m_serialConnBtn = new QPushButton("连接串口", this);
    m_serialConnBtn->setFixedHeight(35);
    m_virtualSerialBtn = new QPushButton("启动模拟器(COM5)", this);
    m_virtualSerialBtn->setFixedHeight(35);
    m_virtualSerialBtn->setToolTip("启动 docs/virtual_serial_device.py，占用 COM5 发送数据；上位机请连接配对端 COM6");

    QHBoxLayout* connLayout = new QHBoxLayout();
    connLayout->addWidget(m_connLabel);
    connLayout->addStretch();
    connLayout->addWidget(m_tcpConnBtn);
    connLayout->addWidget(m_serialConnBtn);
    connLayout->addWidget(m_virtualSerialBtn);
    mainLayout->addLayout(connLayout);

    m_chartWidget = new ChartWidget(this);
    m_chartWidget->setMinimumHeight(300);
    mainLayout->addWidget(m_chartWidget);

    QLabel* alarmTitle = new QLabel("报警记录", this);
    m_alarmList = new QListWidget(this);
    m_alarmList->setMaximumHeight(120);
    mainLayout->addWidget(alarmTitle);
    mainLayout->addWidget(m_alarmList);

    mainLayout->addStretch();

    m_startStopBtn = new QPushButton("开始采集", this);
    m_startStopBtn->setFixedHeight(40);
    mainLayout->addWidget(m_startStopBtn);

    connect(m_startStopBtn, &QPushButton::clicked,
            this, &MainWindow::onStartStopClicked);
    connect(m_tcpConnBtn, &QPushButton::clicked,
            this, &MainWindow::onTcpConnectClicked);
    connect(m_serialConnBtn, &QPushButton::clicked,
            this, &MainWindow::onSerialConnect);
    connect(m_virtualSerialBtn, &QPushButton::clicked,
            this, &MainWindow::onVirtualSerialToggle);
}

void MainWindow::connectSignals()
{
    connect(m_controller, &DeviceMonitorController::dataReceived,
            this, &MainWindow::onDataReceived);
    connect(m_controller, &DeviceMonitorController::alarmTriggered,
            this, &MainWindow::onAlarmTriggered);
    connect(m_controller, &DeviceMonitorController::alarmCleared,
            this, &MainWindow::onAlarmCleared);
    connect(m_controller, &DeviceMonitorController::dataSourceChanged,
            this, &MainWindow::onDataSourceChanged);
    connect(m_controller, &DeviceMonitorController::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    connect(m_controller, &DeviceMonitorController::communicationError,
            this, &MainWindow::onCommunicationError);
    connect(m_controller, &DeviceMonitorController::databaseError,
            this, &MainWindow::onDatabaseError);
    connect(m_controller, &DeviceMonitorController::recordCountChanged,
            this, &MainWindow::updateStatusBar);
    connect(m_controller, &DeviceMonitorController::virtualSerialDeviceStateChanged,
            this, &MainWindow::onVirtualSerialStateChanged);
}

void MainWindow::initializeServices()
{
    const QString dbPath =
        QCoreApplication::applicationDirPath() + "/userdata/QtDeviceMonitor.db";
    if (!m_controller->initialize(dbPath)) {
        QMessageBox::warning(this, "数据库初始化失败",
                             "历史数据和报警记录可能无法保存。\n" + dbPath);
    }
    updateControlsForDataSource(m_controller->dataSource());
    updateStatusBar();
}

void MainWindow::onDataReceived(const DeviceData& data)
{
    m_tempLabel->setText(QString("温度: %1 ℃").arg(data.temperature, 0, 'f', 1));
    m_humLabel->setText(QString("湿度: %1 %").arg(data.humidity, 0, 'f', 1));
    m_pressLabel->setText(QString("压力: %1 MPa").arg(data.pressure, 0, 'f', 4));
    m_co2Label->setText(QString("CO₂: %1 ppm").arg(data.co2, 0, 'f', 0));

    if (data.doorOpen) {
        m_doorLabel->setText("门状态: 开启");
        m_doorLabel->setStyleSheet("color: orange;");
    } else {
        m_doorLabel->setText("门状态: 关闭");
        m_doorLabel->setStyleSheet("color: green;");
    }

    m_timeLabel->setText(QString("时间: %1").arg(data.timestamp.toString("hh:mm:ss")));

    if (data.statusCode == "ALARM") {
        m_statusLabel->setText("状态: ALARM");
        m_statusLabel->setStyleSheet("color: red;");
    } else if (data.statusCode == "WARN") {
        m_statusLabel->setText("状态: WARN");
        m_statusLabel->setStyleSheet("color: orange;");
    } else {
        m_statusLabel->setText("状态: OK");
        m_statusLabel->setStyleSheet("color: green;");
    }

    m_chartWidget->addData(data);
}

void MainWindow::onStartStopClicked()
{
    if (m_controller->dataSource() == DataSource::Mock) {
        m_controller->stop();
    } else {
        m_controller->startMock();
    }
}

void MainWindow::onTcpConnectClicked()
{
    if (m_controller->dataSource() == DataSource::TCP) {
        m_controller->stop();
    } else {
        m_controller->startTcp(m_controller->tcpHost(), m_controller->tcpPort());
    }
}

void MainWindow::onSerialConnect()
{
    if (m_controller->dataSource() == DataSource::Serial) {
        m_controller->stop();
        return;
    }

    SerialConfigDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        m_controller->startSerial(dlg.getConfig());
    }
}

void MainWindow::onVirtualSerialToggle()
{
    if (m_controller->isVirtualSerialDeviceRunning()) {
        m_controller->stopVirtualSerialDevice();
    } else {
        m_controller->startVirtualSerialDevice("COM5", 9600, 100);
    }
}

void MainWindow::onAlarmTriggered(const AlarmEvent& event)
{
    QString item = QString("[%1] %2")
                       .arg(event.timestamp.toString("hh:mm:ss"))
                       .arg(event.message);
    m_alarmList->insertItem(0, item);
    m_alarmList->item(0)->setForeground(Qt::red);
}

void MainWindow::onAlarmCleared(const QString& channel)
{
    QString item = QString("[%1] %2 恢复正常")
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
        "CSV 文件 (*.csv)");
    if (filePath.isEmpty()) {
        return;
    }

    QDateTime to = QDateTime::currentDateTime();
    QDateTime from = to.addSecs(-3600);

    if (m_controller->databaseManager()->exportToCsv(filePath, from, to)) {
        QMessageBox::information(this, "导出成功",
                                 "数据已导出到：\n" + filePath);
    } else {
        QMessageBox::warning(this, "导出失败", "没有数据或文件写入失败");
    }
}

void MainWindow::onSettings()
{
    SettingsDialog dlg(this);
    dlg.setTempConfig(m_controller->tempConfig());
    dlg.setHumConfig(m_controller->humConfig());
    dlg.setPressConfig(m_controller->pressConfig());
    dlg.setCo2Config(m_controller->co2Config());

    if (dlg.exec() == QDialog::Accepted) {
        m_controller->updateAlarmConfigs(
            dlg.getTempConfig(),
            dlg.getHumConfig(),
            dlg.getPressConfig(),
            dlg.getCo2Config());
    }
}

void MainWindow::onHistory()
{
    HistoryDialog dlg(m_controller->databaseManager(), this);
    dlg.exec();
}

void MainWindow::onAlarmHistory()
{
    AlarmHistoryDialog dlg(m_controller->databaseManager(), this);
    dlg.exec();
}

void MainWindow::onDataSourceChanged(DataSource source)
{
    updateControlsForDataSource(source);
    updateStatusBar();
}

void MainWindow::onConnectionStatusChanged(const QString& text, const QString& colorName)
{
    m_connLabel->setText(text);
    m_connLabel->setStyleSheet("color: " + colorName + ";");
}

void MainWindow::onCommunicationError(const QString& message)
{
    statusBar()->showMessage("通信错误: " + message, 5000);
}

void MainWindow::onDatabaseError(const QString& message)
{
    statusBar()->showMessage(message, 5000);
}

void MainWindow::onVirtualSerialStateChanged(bool running, const QString& message)
{
    m_virtualSerialBtn->setText(running ? "停止模拟器(COM5)" : "启动模拟器(COM5)");
    statusBar()->showMessage(message, 5000);
}

void MainWindow::updateStatusBar()
{
    QString msg = QString("数据源: %1 | 记录: %2")
                      .arg(dataSourceName(m_controller->dataSource()))
                      .arg(m_controller->recordCount());

    if (m_controller->dataSource() != DataSource::None
        && m_controller->collectStartTime().isValid()) {
        qint64 secs = m_controller->collectStartTime().secsTo(QDateTime::currentDateTime());
        msg += QString(" | 运行: %1")
                   .arg(QDateTime::fromSecsSinceEpoch(secs).toUTC().toString("hh:mm:ss"));
    }

    statusBar()->showMessage(msg);
}

void MainWindow::updateControlsForDataSource(DataSource source)
{
    switch (source) {
    case DataSource::None:
        m_startStopBtn->setText("开始采集");
        m_tcpConnBtn->setText("连接虚拟设备(TCP)");
        m_serialConnBtn->setText("连接串口");
        break;
    case DataSource::Mock:
        m_startStopBtn->setText("停止采集");
        m_tcpConnBtn->setText("连接虚拟设备(TCP)");
        m_serialConnBtn->setText("连接串口");
        break;
    case DataSource::TCP:
        m_startStopBtn->setText("开始采集");
        m_tcpConnBtn->setText("断开 TCP");
        m_serialConnBtn->setText("连接串口");
        break;
    case DataSource::Serial:
        m_startStopBtn->setText("开始采集");
        m_tcpConnBtn->setText("连接虚拟设备(TCP)");
        m_serialConnBtn->setText("断开串口");
        break;
    }
}
