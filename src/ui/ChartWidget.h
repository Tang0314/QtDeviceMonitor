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
    void addData(const DeviceData& data);  // 接收新数据点
    void clear();                           // 清空曲线

private:
    void setupChart();

    QChartView*    m_chartView;
    QLineSeries*   m_tempSeries;    // 温度曲线
    QLineSeries*   m_pressSeries;   // 压力曲线
    QDateTimeAxis* m_axisX;         // 时间轴
    QValueAxis*    m_axisTempY;     // 温度Y轴

    static const int MAX_POINTS = 300;  // 最多显示300个点（30秒@100ms）
};
