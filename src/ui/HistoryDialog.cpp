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

    m_queryBtn  = new QPushButton("查询", this);
    m_exportBtn = new QPushButton("导出CSV", this);

    timeLayout->addWidget(new QLabel("从:"));
    timeLayout->addWidget(m_fromEdit);
    timeLayout->addWidget(new QLabel("到:"));
    timeLayout->addWidget(m_toEdit);
    timeLayout->addWidget(m_queryBtn);
    timeLayout->addWidget(m_exportBtn);

    // ── 数据表格 ──
    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "时间", "温度(℃)", "湿度(%)", "压力(MPa)", "CO₂(ppm)", "门状态", "状态"
    });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);

    // 时间列自适应内容宽度（yyyy-MM-dd hh:mm:ss 需要 ~30 个字符宽度）
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    // ── 分页导航 ──
    QHBoxLayout* navLayout = new QHBoxLayout();
    m_prevBtn = new QPushButton("上一页", this);
    m_nextBtn = new QPushButton("下一页", this);
    m_countLabel = new QLabel("共 0 条记录", this);
    m_prevBtn->setEnabled(false);
    m_nextBtn->setEnabled(false);

    navLayout->addWidget(m_countLabel);
    navLayout->addStretch();
    navLayout->addWidget(m_prevBtn);
    navLayout->addWidget(m_nextBtn);

    connect(m_queryBtn,  &QPushButton::clicked, this, &HistoryDialog::onQuery);
    connect(m_exportBtn, &QPushButton::clicked, this, &HistoryDialog::onExport);
    connect(m_prevBtn,   &QPushButton::clicked, this, &HistoryDialog::onPrevPage);
    connect(m_nextBtn,   &QPushButton::clicked, this, &HistoryDialog::onNextPage);

    mainLayout->addWidget(timeGroup);
    mainLayout->addWidget(m_table);
    mainLayout->addLayout(navLayout);
}

void HistoryDialog::onQuery()
{
    if (m_fromEdit->dateTime() >= m_toEdit->dateTime()) {
        QMessageBox::warning(this, "时间错误", "开始时间必须早于结束时间！");
        return;
    }

    int total = m_dbManager->countByTimeRange(
        m_fromEdit->dateTime(), m_toEdit->dateTime());
    m_totalPages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    m_currentPage = 0;

    if (m_totalPages == 0) {
        m_table->setRowCount(0);
        m_countLabel->setText("共 0 条记录");
        m_prevBtn->setEnabled(false);
        m_nextBtn->setEnabled(false);
        return;
    }

    refreshTable();
}

void HistoryDialog::refreshTable()
{
    int offset = m_currentPage * PAGE_SIZE;
    auto dataList = m_dbManager->queryByTimeRange(
        m_fromEdit->dateTime(), m_toEdit->dateTime(),
        offset, PAGE_SIZE);

    m_table->setRowCount(0);
    for (const auto& data : dataList) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(
                                     data.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
        m_table->setItem(row, 1, new QTableWidgetItem(
                                     QString::number(data.temperature, 'f', 1)));
        m_table->setItem(row, 2, new QTableWidgetItem(
                                     QString::number(data.humidity, 'f', 1)));
        m_table->setItem(row, 3, new QTableWidgetItem(
                                     QString::number(data.pressure, 'f', 4)));
        m_table->setItem(row, 4, new QTableWidgetItem(
                                     QString::number(data.co2, 'f', 0)));

        QTableWidgetItem* doorItem = new QTableWidgetItem(
            data.doorOpen ? "⚠ 开启" : "✓ 关闭");
        doorItem->setForeground(data.doorOpen ? QColor("orange") : Qt::darkGreen);
        m_table->setItem(row, 5, doorItem);

        QTableWidgetItem* statusItem = new QTableWidgetItem(data.statusCode);
        if (data.statusCode == "ALARM") {
            statusItem->setForeground(Qt::red);
        } else if (data.statusCode == "WARN") {
            statusItem->setForeground(QColor("orange"));
        } else {
            statusItem->setForeground(Qt::darkGreen);
        }
        m_table->setItem(row, 6, statusItem);
    }

    int total = m_dbManager->countByTimeRange(
        m_fromEdit->dateTime(), m_toEdit->dateTime());
    m_countLabel->setText(QString("第 %1/%2 页（共 %3 条）")
                              .arg(m_currentPage + 1)
                              .arg(m_totalPages)
                              .arg(total));

    m_prevBtn->setEnabled(m_currentPage > 0);
    m_nextBtn->setEnabled(m_currentPage < m_totalPages - 1);
}

void HistoryDialog::onPrevPage()
{
    if (m_currentPage > 0) {
        m_currentPage--;
        refreshTable();
    }
}

void HistoryDialog::onNextPage()
{
    if (m_currentPage < m_totalPages - 1) {
        m_currentPage++;
        refreshTable();
    }
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
