#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include "data/DatabaseManager.h"

class AlarmHistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit AlarmHistoryDialog(DatabaseManager* dbManager,
                                 QWidget* parent = nullptr);

private slots:
    void onQuery();

private:
    void setupUI();

    DatabaseManager* m_dbManager;

    QDateTimeEdit* m_fromEdit;
    QDateTimeEdit* m_toEdit;
    QTableWidget*  m_table;
    QPushButton*   m_queryBtn;
    QLabel*        m_countLabel;
};
