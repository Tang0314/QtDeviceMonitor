#include "ui/HistoryDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>


HistoryDialog::HistoryDialog(DatabaseManager* dbManager, QWidget* parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setupUI();
}

void HistoryDialog::setupUI()
{
    setWindowTitle("历史数据查询");
    setMinimumSize(700, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ── 时间范围选择 ──
    QGroupBox* timeGroup = new QGroupBox("查询时间范围", this);
    QHBoxLayout* timeLayout = new QHBoxLayout(timeGroup);

    m_fromEdit = new QDateTimeEdit(this);
    m_fromEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_fromEdit->setDateTime(QDateTime::currentDateTime().addSecs(-3600));
    m_fromEdit->setCalendarPopup(true);

    m_toEdit = new QDateTimeEdit(this);
    m_toEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_toEdit->setDateTime(QDateTime::currentDateTime());
    m_toEdit->setCalendarPopup(true);

    m_queryBtn  = new QPushButton("🔍 查询", this);
    m_exportBtn = new QPushButton("📄 导出CSV", this);

    timeLayout->addWidget(new QLabel("从:"));
    timeLayout->addWidget(m_fromEdit);
    timeLayout->addWidget(new QLabel("到:"));
    timeLayout->addWidget(m_toEdit);
    timeLayout->addWidget(m_queryBtn);
    timeLayout->addWidget(m_exportBtn);

    // ── 数据表格 ──
    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"时间", "温度(℃)", "压力(MPa)", "状态"});
    m_table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);

    // ── 状态栏 ──
    m_countLabel = new QLabel("共 0 条记录", this);

    connect(m_queryBtn,  &QPushButton::clicked, this, &HistoryDialog::onQuery);
    connect(m_exportBtn, &QPushButton::clicked, this, &HistoryDialog::onExport);

    mainLayout->addWidget(timeGroup);
    mainLayout->addWidget(m_table);
    mainLayout->addWidget(m_countLabel);
}

void HistoryDialog::onQuery()
{
    if (m_fromEdit->dateTime() >= m_toEdit->dateTime()) {
        QMessageBox::warning(this, "时间错误", "开始时间必须早于结束时间！");
        return;
    }

    auto dataList = m_dbManager->queryByTimeRange(
        m_fromEdit->dateTime(),
        m_toEdit->dateTime()
        );

    m_table->setRowCount(0);
    for (const auto& data : dataList) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(
                                     data.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
        m_table->setItem(row, 1, new QTableWidgetItem(
                                     QString::number(data.temperature, 'f', 1)));
        m_table->setItem(row, 2, new QTableWidgetItem(
                                     QString::number(data.pressure, 'f', 2)));

        QTableWidgetItem* statusItem = new QTableWidgetItem(data.statusCode);
        if (data.statusCode == "ALARM") {
            statusItem->setForeground(Qt::red);
        } else if (data.statusCode == "WARN") {
            statusItem->setForeground(QColor("orange"));
        } else {
            statusItem->setForeground(Qt::darkGreen);
        }
        m_table->setItem(row, 3, statusItem);
    }

    m_countLabel->setText(QString("共 %1 条记录").arg(dataList.size()));
}

void HistoryDialog::onExport()
{
    if (m_fromEdit->dateTime() >= m_toEdit->dateTime()) {
        QMessageBox::warning(this, "时间错误", "开始时间必须早于结束时间！");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "导出CSV",
        QCoreApplication::applicationDirPath() + "/userdata/history_export.csv",
        "CSV 文件 (*.csv)"
        );
    if (filePath.isEmpty()) return;

    if (m_dbManager->exportToCsv(filePath,
                                 m_fromEdit->dateTime(), m_toEdit->dateTime())) {
        QMessageBox::information(this, "导出成功",
                                 "数据已导出到：\n" + filePath);
    } else {
        QMessageBox::warning(this, "导出失败", "没有数据或文件写入失败");
    }
}
