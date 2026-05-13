#pragma once

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include "comm/SerialComm.h"

class SerialConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit SerialConfigDialog(QWidget* parent = nullptr);

    SerialConfig getConfig() const;

private slots:
    void onRefreshPorts();
    void onConfirm();

private:
    void setupUI();

    QComboBox*   m_portCombo;      // 串口号
    QComboBox*   m_baudCombo;      // 波特率
    QComboBox*   m_dataBitsCombo;  // 数据位
    QComboBox*   m_stopBitsCombo;  // 停止位
    QComboBox*   m_parityCombo;    // 校验位
    QPushButton* m_refreshBtn;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
};
