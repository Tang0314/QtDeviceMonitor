#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include "alarm/AlarmChecker.h"

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // 获取用户设置的配置
    AlarmConfig getTempConfig() const;
    AlarmConfig getPressConfig() const;

    // 设置初始值
    void setTempConfig(const AlarmConfig& config);
    void setPressConfig(const AlarmConfig& config);

private slots:
    void onConfirm();

private:
    void setupUI();

    // 温度报警设置
    QDoubleSpinBox* m_tempHighSpin;
    QDoubleSpinBox* m_tempLowSpin;
    QCheckBox*      m_tempEnabledCheck;

    // 压力报警设置
    QDoubleSpinBox* m_pressHighSpin;
    QDoubleSpinBox* m_pressLowSpin;
    QCheckBox*      m_pressEnabledCheck;

    QPushButton*    m_confirmBtn;
    QPushButton*    m_cancelBtn;
};
