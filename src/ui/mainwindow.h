#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "mock/MockDataGenerator.h"
#include "mock/VirtualTcpDevice.h"
#include "comm/TcpComm.h"
#include "ui/ChartWidget.h"
#include "alarm/AlarmChecker.h"
#include "data/DatabaseManager.h"
#include "ui/SettingsDialog.h"
#include "ui/HistoryDialog.h"
#include "config/ConfigManager.h"
#include "comm/SerialComm.h"
#include "ui/SerialConfigDialog.h"

enum class DataSource {
    None,
    Mock,
    TCP,
    Serial
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onDataGenerated(const DeviceData& data);
    void onStartStopClicked();
    void onAlarmTriggered(const AlarmEvent& event);
    void onAlarmCleared(const QString& channel);
    void onExportCsv();
    void onSettings();
    void onHistory();
    void onAlarmHistory();
    void onTcpStateChanged(bool connected);
    void onTcpError(const QString& msg);
    void onSerialConnect();
    void onSerialStateChanged(bool connected);

private:
    void setupUI();
    void connectSignals();

    // 数据源状态机：确保 Mock/TCP/Serial 互斥
    void setDataSource(DataSource source);
    void updateStatusBar();

    // UI 控件
    QLabel*      m_tempLabel;
    QLabel*      m_humLabel;       // 湿度
    QLabel*      m_pressLabel;
    QLabel*      m_co2Label;       // CO₂
    QLabel*      m_doorLabel;      // 门状态
    QLabel*      m_statusLabel;
    QLabel*      m_timeLabel;
    QLabel*      m_connLabel;
    QPushButton* m_startStopBtn;
    QPushButton* m_tcpConnBtn;
    ChartWidget* m_chartWidget;
    QListWidget* m_alarmList;

    // 核心模块
    MockDataGenerator* m_mockGenerator;
    VirtualTcpDevice*  m_virtualDevice;
    TcpComm*           m_tcpComm;
    AlarmChecker*      m_alarmChecker;
    DatabaseManager*   m_dbManager;

    AlarmConfig m_tempConfig;
    AlarmConfig m_humConfig;
    AlarmConfig m_pressConfig;
    AlarmConfig m_co2Config;

    ConfigManager m_configManager;

    DataSource m_dataSource = DataSource::None;

    // 状态栏统计
    int       m_recordCount = 0;
    QDateTime m_collectStartTime;

    SerialComm* m_serialComm;
    QPushButton* m_serialConnBtn;   // 串口连接按钮
};
