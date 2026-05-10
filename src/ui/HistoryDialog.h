#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include "data/DatabaseManager.h"

class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistoryDialog(DatabaseManager* dbManager,
                           QWidget* parent = nullptr);

private slots:
    void onQuery();
    void onExport();

private:
    void setupUI();

    DatabaseManager* m_dbManager;

    QDateTimeEdit* m_fromEdit;    // 开始时间
    QDateTimeEdit* m_toEdit;      // 结束时间
    QTableWidget*  m_table;       // 数据表格
    QPushButton*   m_queryBtn;
    QPushButton*   m_exportBtn;
    QLabel*        m_countLabel;  // 显示查询到多少条
};
