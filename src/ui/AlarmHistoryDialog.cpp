#include "ui/AlarmHistoryDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

AlarmHistoryDialog::AlarmHistoryDialog(DatabaseManager* dbManager, QWidget* parent)
    : QDialog(parent)
    , m_dbManager(dbManager)
{
    setupUI();
}

void AlarmHistoryDialog::setupUI()
{
    setWindowTitle("报警历史查询");
    setMinimumSize(750, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ── 时间范围 ──
    QGroupBox* timeGroup = new QGroupBox("查询时间范围", this);
    QHBoxLayout* timeLayout = new QHBoxLayout(timeGroup);

    m_fromEdit = new QDateTimeEdit(this);
    m_fromEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_fromEdit->setDateTime(QDateTime::currentDateTime().addSecs(-86400));  // 默认最近24h
    m_fromEdit->setCalendarPopup(true);

    m_toEdit = new QDateTimeEdit(this);
    m_toEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    m_toEdit->setDateTime(QDateTime::currentDateTime());
    m_toEdit->setCalendarPopup(true);

    m_queryBtn = new QPushButton("查询", this);

    timeLayout->addWidget(new QLabel("从:"));
    timeLayout->addWidget(m_fromEdit);
    timeLayout->addWidget(new QLabel("到:"));
    timeLayout->addWidget(m_toEdit);
    timeLayout->addWidget(m_queryBtn);
    timeLayout->addStretch();

    // ── 表格 ──
    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({
        "时间", "通道", "类型", "当前值", "阈值", "消息"
    });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    // ── 状态栏 ──
    m_countLabel = new QLabel("共 0 条记录", this);

    connect(m_queryBtn, &QPushButton::clicked, this, &AlarmHistoryDialog::onQuery);

    mainLayout->addWidget(timeGroup);
    mainLayout->addWidget(m_table);
    mainLayout->addWidget(m_countLabel);
}

void AlarmHistoryDialog::onQuery()
{
    if (m_fromEdit->dateTime() >= m_toEdit->dateTime()) {
        QMessageBox::warning(this, "时间错误", "开始时间必须早于结束时间！");
        return;
    }

    auto alarmList = m_dbManager->queryAlarmsByTimeRange(
        m_fromEdit->dateTime(),
        m_toEdit->dateTime()
        );

    m_table->setRowCount(0);
    for (const auto& evt : alarmList) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, 0, new QTableWidgetItem(
                                     evt.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
        m_table->setItem(row, 1, new QTableWidgetItem(evt.channel));

        QString typeStr = (evt.type == AlarmEvent::Type::HighLimit) ? "超上限" : "低于下限";
        QTableWidgetItem* typeItem = new QTableWidgetItem(typeStr);
        typeItem->setForeground(Qt::red);
        m_table->setItem(row, 2, typeItem);

        m_table->setItem(row, 3, new QTableWidgetItem(
                                     QString::number(evt.value, 'f', 2)));
        m_table->setItem(row, 4, new QTableWidgetItem(
                                     QString::number(evt.limit, 'f', 2)));
        m_table->setItem(row, 5, new QTableWidgetItem(evt.message));
    }

    m_countLabel->setText(QString("共 %1 条记录").arg(alarmList.size()));
}
