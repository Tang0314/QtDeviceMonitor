#include "ui/ChartWidget.h"
#include <QVBoxLayout>

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
{
    setupChart();
}

void ChartWidget::setupChart()
{
    m_tempSeries = new QLineSeries(this);
    m_humSeries  = new QLineSeries(this);
    m_co2Series  = new QLineSeries(this);

    m_tempSeries->setName("温度 (℃)");
    m_humSeries->setName("湿度 (%)");
    m_co2Series->setName("CO₂ (ppm)");

    m_tempSeries->setColor(Qt::red);
    m_humSeries->setColor(Qt::blue);
    m_co2Series->setColor(Qt::darkGreen);

    QChart* chart = new QChart();
    chart->addSeries(m_tempSeries);
    chart->addSeries(m_humSeries);
    chart->addSeries(m_co2Series);
    chart->setTitle("冷链仓储实时监控曲线");
    chart->setAnimationOptions(QChart::NoAnimation);

    // 时间轴
    m_axisX = new QDateTimeAxis(this);
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("时间");
    m_axisX->setTickCount(6);
    chart->addAxis(m_axisX, Qt::AlignBottom);

    // 左Y轴（温度 -30~-10℃）
    m_axisTempY = new QValueAxis(this);
    m_axisTempY->setRange(-30, -10);
    m_axisTempY->setTitleText("温度 (℃)");
    chart->addAxis(m_axisTempY, Qt::AlignLeft);

    // 右Y轴（湿度 0~100%）
    m_axisHumY = new QValueAxis(this);
    m_axisHumY->setRange(0, 100);
    m_axisHumY->setTitleText("湿度 (%)");
    chart->addAxis(m_axisHumY, Qt::AlignRight);

    // 右Y轴（CO₂ 0~2000ppm）
    m_axisCo2Y = new QValueAxis(this);
    m_axisCo2Y->setRange(0, 2000);
    m_axisCo2Y->setTitleText("CO₂ (ppm)");
    chart->addAxis(m_axisCo2Y, Qt::AlignRight);

    // 绑定轴
    m_tempSeries->attachAxis(m_axisX);
    m_tempSeries->attachAxis(m_axisTempY);
    m_humSeries->attachAxis(m_axisX);
    m_humSeries->attachAxis(m_axisHumY);
    m_co2Series->attachAxis(m_axisX);
    m_co2Series->attachAxis(m_axisCo2Y);

    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);
}

void ChartWidget::addData(const DeviceData& data)
{
    qint64 msec = data.timestamp.toMSecsSinceEpoch();

    m_tempSeries->append(msec, data.temperature);
    m_humSeries->append(msec, data.humidity);
    m_co2Series->append(msec, data.co2);

    if (m_tempSeries->count() > MAX_POINTS) {
        m_tempSeries->remove(0);
        m_humSeries->remove(0);
        m_co2Series->remove(0);
    }

    if (m_tempSeries->count() >= 2) {
        QDateTime tMin = QDateTime::fromMSecsSinceEpoch(
            m_tempSeries->at(0).x());
        QDateTime tMax = QDateTime::fromMSecsSinceEpoch(
            m_tempSeries->at(m_tempSeries->count() - 1).x());
        m_axisX->setRange(tMin, tMax);
    }
}

void ChartWidget::clear()
{
    m_tempSeries->clear();
    m_humSeries->clear();
    m_co2Series->clear();
}
