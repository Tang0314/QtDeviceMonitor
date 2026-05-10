#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "mock/MockDataGenerator.h"
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

private:
    void setupUI();

    QLabel*      m_tempLabel;
    QLabel*      m_pressLabel;
    QLabel*      m_statusLabel;
    QLabel*      m_timeLabel;
    QPushButton* m_startStopBtn;
    ChartWidget* m_chartWidget;
    QListWidget* m_alarmList;

    MockDataGenerator* m_mockGenerator;
    AlarmChecker*      m_alarmChecker;
    DatabaseManager*   m_dbManager;

    AlarmConfig m_tempConfig;
    AlarmConfig m_pressConfig;
};
