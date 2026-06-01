#pragma once

#include "alarm/AlarmChecker.h"
#include "core/DeviceMonitorController.h"
#include "data/DeviceData.h"
#include "ui/ChartWidget.h"

#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onDataReceived(const DeviceData& data);
    void onStartStopClicked();
    void onTcpConnectClicked();
    void onSerialConnect();
    void onVirtualSerialToggle();
    void onAlarmTriggered(const AlarmEvent& event);
    void onAlarmCleared(const QString& channel);
    void onExportCsv();
    void onSettings();
    void onHistory();
    void onAlarmHistory();
    void onDataSourceChanged(DataSource source);
    void onConnectionStatusChanged(const QString& text, const QString& colorName);
    void onCommunicationError(const QString& message);
    void onDatabaseError(const QString& message);
    void onVirtualSerialStateChanged(bool running, const QString& message);

private:
    void setupUI();
    void connectSignals();
    void initializeServices();
    void updateStatusBar();
    void updateControlsForDataSource(DataSource source);

    QLabel*      m_tempLabel = nullptr;
    QLabel*      m_humLabel = nullptr;
    QLabel*      m_pressLabel = nullptr;
    QLabel*      m_co2Label = nullptr;
    QLabel*      m_doorLabel = nullptr;
    QLabel*      m_statusLabel = nullptr;
    QLabel*      m_timeLabel = nullptr;
    QLabel*      m_connLabel = nullptr;
    QPushButton* m_startStopBtn = nullptr;
    QPushButton* m_tcpConnBtn = nullptr;
    QPushButton* m_serialConnBtn = nullptr;
    QPushButton* m_virtualSerialBtn = nullptr;
    ChartWidget* m_chartWidget = nullptr;
    QListWidget* m_alarmList = nullptr;

    DeviceMonitorController* m_controller = nullptr;
};
