#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include "alarm/AlarmChecker.h"
#include <QGroupBox>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    AlarmConfig getTempConfig()  const;
    AlarmConfig getHumConfig()   const;
    AlarmConfig getPressConfig() const;
    AlarmConfig getCo2Config()   const;

    void setTempConfig(const AlarmConfig& config);
    void setHumConfig(const AlarmConfig& config);
    void setPressConfig(const AlarmConfig& config);
    void setCo2Config(const AlarmConfig& config);

private slots:
    void onConfirm();

private:
    void setupUI();
    QGroupBox* createChannelGroup(
        const QString& title,
        QDoubleSpinBox*& highSpin,
        QDoubleSpinBox*& lowSpin,
        QCheckBox*& enabledCheck,
        double highVal, double lowVal,
        double min, double max,
        double step, int decimals,
        const QString& suffix
        );

    // 温度
    QDoubleSpinBox* m_tempHighSpin;
    QDoubleSpinBox* m_tempLowSpin;
    QCheckBox*      m_tempEnabledCheck;

    // 湿度
    QDoubleSpinBox* m_humHighSpin;
    QDoubleSpinBox* m_humLowSpin;
    QCheckBox*      m_humEnabledCheck;

    // 压力
    QDoubleSpinBox* m_pressHighSpin;
    QDoubleSpinBox* m_pressLowSpin;
    QCheckBox*      m_pressEnabledCheck;

    // CO₂
    QDoubleSpinBox* m_co2HighSpin;
    QDoubleSpinBox* m_co2LowSpin;
    QCheckBox*      m_co2EnabledCheck;

    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
};
