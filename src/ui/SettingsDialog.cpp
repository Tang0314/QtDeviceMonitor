#include "ui/SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
}

void SettingsDialog::setupUI()
{
    setWindowTitle("设置 - 报警阈值");
    setMinimumWidth(300);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ── 温度设置组 ──
    QGroupBox* tempGroup = new QGroupBox("温度报警设置", this);
    QGridLayout* tempGrid = new QGridLayout(tempGroup);
    m_tempEnabledCheck = new QCheckBox("启用温度报警", this);
    m_tempEnabledCheck->setChecked(true);
    m_tempHighSpin = new QDoubleSpinBox(this);
    m_tempHighSpin->setRange(-100, 500);
    m_tempHighSpin->setValue(28.0);
    m_tempHighSpin->setSuffix(" ℃");
    m_tempHighSpin->setDecimals(1);
    m_tempLowSpin = new QDoubleSpinBox(this);
    m_tempLowSpin->setRange(-100, 500);
    m_tempLowSpin->setValue(15.0);
    m_tempLowSpin->setSuffix(" ℃");
    m_tempLowSpin->setDecimals(1);
    tempGrid->addWidget(m_tempEnabledCheck, 0, 0, 1, 2);
    tempGrid->addWidget(new QLabel("上限:"), 1, 0);
    tempGrid->addWidget(m_tempHighSpin,     1, 1);
    tempGrid->addWidget(new QLabel("下限:"), 2, 0);
    tempGrid->addWidget(m_tempLowSpin,      2, 1);

    // ── 压力设置组 ──
    QGroupBox* pressGroup = new QGroupBox("压力报警设置", this);
    QGridLayout* pressGrid = new QGridLayout(pressGroup);
    m_pressEnabledCheck = new QCheckBox("启用压力报警", this);
    m_pressEnabledCheck->setChecked(true);
    m_pressHighSpin = new QDoubleSpinBox(this);
    m_pressHighSpin->setRange(0, 100);
    m_pressHighSpin->setValue(1.1);   // 改小，让压力能触发报警
    m_pressHighSpin->setSuffix(" MPa");
    m_pressHighSpin->setDecimals(2);
    m_pressHighSpin->setSingleStep(0.05);
    m_pressLowSpin = new QDoubleSpinBox(this);
    m_pressLowSpin->setRange(0, 100);
    m_pressLowSpin->setValue(0.98);   // 改大，让压力能触发报警
    m_pressLowSpin->setSuffix(" MPa");
    m_pressLowSpin->setDecimals(2);
    m_pressLowSpin->setSingleStep(0.05);
    pressGrid->addWidget(m_pressEnabledCheck, 0, 0, 1, 2);
    pressGrid->addWidget(new QLabel("上限:"),  1, 0);
    pressGrid->addWidget(m_pressHighSpin,      1, 1);
    pressGrid->addWidget(new QLabel("下限:"),  2, 0);
    pressGrid->addWidget(m_pressLowSpin,       2, 1);

    // ── 按钮 ──
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_confirmBtn = new QPushButton("确定", this);
    m_cancelBtn  = new QPushButton("取消", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);
    connect(m_confirmBtn, &QPushButton::clicked, this, &SettingsDialog::onConfirm);
    connect(m_cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);

    mainLayout->addWidget(tempGroup);
    mainLayout->addWidget(pressGroup);
    mainLayout->addLayout(btnLayout);
}

AlarmConfig SettingsDialog::getTempConfig() const
{
    AlarmConfig config;
    config.highLimit = m_tempHighSpin->value();
    config.lowLimit  = m_tempLowSpin->value();
    config.enabled   = m_tempEnabledCheck->isChecked();
    return config;
}

AlarmConfig SettingsDialog::getPressConfig() const
{
    AlarmConfig config;
    config.highLimit = m_pressHighSpin->value();
    config.lowLimit  = m_pressLowSpin->value();
    config.enabled   = m_pressEnabledCheck->isChecked();
    return config;
}

void SettingsDialog::setTempConfig(const AlarmConfig& config)
{
    m_tempHighSpin->setValue(config.highLimit);
    m_tempLowSpin->setValue(config.lowLimit);
    m_tempEnabledCheck->setChecked(config.enabled);
}

void SettingsDialog::setPressConfig(const AlarmConfig& config)
{
    m_pressHighSpin->setValue(config.highLimit);
    m_pressLowSpin->setValue(config.lowLimit);
    m_pressEnabledCheck->setChecked(config.enabled);
}

void SettingsDialog::onConfirm()
{
    if (m_tempHighSpin->value() <= m_tempLowSpin->value()) {
        QMessageBox::warning(this, "设置错误", "温度上限必须大于下限！");
        return;
    }
    if (m_pressHighSpin->value() <= m_pressLowSpin->value()) {
        QMessageBox::warning(this, "设置错误", "压力上限必须大于下限！");
        return;
    }
    accept();
}
