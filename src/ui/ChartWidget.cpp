#include "ui/ChartWidget.h"
#include <QVBoxLayout>

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupChart();
}

void ChartWidget::setupChart()
{
    m_tempSeries  = new QLineSeries(this);
    m_pressSeries = new QLineSeries(this);

    m_tempSeries->setName("温度 (℃)");
    m_pressSeries->setName("压力 (MPa)");
    m_tempSeries->setColor(Qt::red);
    m_pressSeries->setColor(Qt::blue);

    QChart* chart = new QChart();
    chart->addSeries(m_tempSeries);
    chart->addSeries(m_pressSeries);
    chart->setTitle("实时数据曲线");
    chart->setAnimationOptions(QChart::NoAnimation);

    // 时间轴（X轴）
    m_axisX = new QDateTimeAxis(this);
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("时间");
    m_axisX->setTickCount(6);
    chart->addAxis(m_axisX, Qt::AlignBottom);

    // 温度Y轴（左侧）
    m_axisTempY = new QValueAxis(this);
    m_axisTempY->setRange(0, 40);
    m_axisTempY->setTitleText("温度 (℃)");
    chart->addAxis(m_axisTempY, Qt::AlignLeft);

    // 压力Y轴（右侧）
    m_axisPressY = new QValueAxis(this);
    m_axisPressY->setRange(0.8, 1.3);
    m_axisPressY->setTitleText("压力 (MPa)");
    chart->addAxis(m_axisPressY, Qt::AlignRight);

    // 绑定轴
    m_tempSeries->attachAxis(m_axisX);
    m_tempSeries->attachAxis(m_axisTempY);
    m_pressSeries->attachAxis(m_axisX);
    m_pressSeries->attachAxis(m_axisPressY);

    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
}

void ChartWidget::clear()
{
    m_tempSeries->clear();
    m_pressSeries->clear();
}

void ChartWidget::addData(const DeviceData& data)
{
    qint64 msec = data.timestamp.toMSecsSinceEpoch();

    m_tempSeries->append(msec, data.temperature);
    m_pressSeries->append(msec, data.pressure);

    if (m_tempSeries->count() > MAX_POINTS) {
        m_tempSeries->remove(0);
        m_pressSeries->remove(0);
    }

    if (m_tempSeries->count() >= 2) {
        QDateTime tMin = QDateTime::fromMSecsSinceEpoch(
            m_tempSeries->at(0).x());
        QDateTime tMax = QDateTime::fromMSecsSinceEpoch(
            m_tempSeries->at(m_tempSeries->count() - 1).x());
        m_axisX->setRange(tMin, tMax);
    }
}
