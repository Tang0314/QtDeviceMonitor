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
    void onTcpStateChanged(bool connected);
    void onTcpError(const QString& msg);

private:
    void setupUI();
    void connectSignals();

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
    AlarmConfig m_pressConfig;

    bool m_useTcp = false;  // 当前使用TCP还是Mock
};
