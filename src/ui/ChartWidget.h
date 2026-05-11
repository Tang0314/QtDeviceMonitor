#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include "data/DeviceData.h"

class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);

public slots:
    void addData(const DeviceData& data);
    void clear();

private:
    void setupChart();

    QChartView*    m_chartView;
    QLineSeries*   m_tempSeries;    // 温度
    QLineSeries*   m_humSeries;     // 湿度
    QLineSeries*   m_co2Series;     // CO₂
    QDateTimeAxis* m_axisX;
    QValueAxis*    m_axisTempY;     // 温度Y轴（左）
    QValueAxis*    m_axisHumY;      // 湿度Y轴（右上）
    QValueAxis*    m_axisCo2Y;      // CO₂Y轴（右）

    static const int MAX_POINTS = 300;
};
