#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "mock/MockDataGenerator.h"
#include "ui/ChartWidget.h"

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

    QLabel*      m_tempLabel;
    QLabel*      m_pressLabel;
    QLabel*      m_statusLabel;
    QLabel*      m_timeLabel;
    QPushButton* m_startStopBtn;
    ChartWidget* m_chartWidget;

    MockDataGenerator* m_mockGenerator;
};
