#include "ui/SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
}

QGroupBox* SettingsDialog::createChannelGroup(
    const QString& title,
    QDoubleSpinBox*& highSpin,
    QDoubleSpinBox*& lowSpin,
    QCheckBox*& enabledCheck,
    double highVal, double lowVal,
    double min, double max,
    double step, int decimals,
    const QString& suffix)
{
    QGroupBox* group = new QGroupBox(title, this);
    QGridLayout* grid = new QGridLayout(group);

    enabledCheck = new QCheckBox("启用报警", this);
    enabledCheck->setChecked(true);

    highSpin = new QDoubleSpinBox(this);
    highSpin->setRange(min, max);
    highSpin->setValue(highVal);
    highSpin->setSuffix(suffix);
    highSpin->setDecimals(decimals);
    highSpin->setSingleStep(step);

    lowSpin = new QDoubleSpinBox(this);
    lowSpin->setRange(min, max);
    lowSpin->setValue(lowVal);
    lowSpin->setSuffix(suffix);
    lowSpin->setDecimals(decimals);
    lowSpin->setSingleStep(step);

    grid->addWidget(enabledCheck,        0, 0, 1, 2);
    grid->addWidget(new QLabel("上限:"), 1, 0);
    grid->addWidget(highSpin,            1, 1);
    grid->addWidget(new QLabel("下限:"), 2, 0);
    grid->addWidget(lowSpin,             2, 1);

    return group;
}

void SettingsDialog::setupUI()
{
    setWindowTitle("设置 - 报警阈值");
    setMinimumWidth(350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 两列布局
    QGridLayout* groupGrid = new QGridLayout();

    groupGrid->addWidget(createChannelGroup(
                             "🌡 温度报警", m_tempHighSpin, m_tempLowSpin, m_tempEnabledCheck,
                             -15.0, -23.0, -50.0, 50.0, 0.5, 1, " ℃"), 0, 0);

    groupGrid->addWidget(createChannelGroup(
                             "💧 湿度报警", m_humHighSpin, m_humLowSpin, m_humEnabledCheck,
                             95.0, 60.0, 0.0, 100.0, 1.0, 1, " %"), 0, 1);

    groupGrid->addWidget(createChannelGroup(
                             "🔵 压力报警", m_pressHighSpin, m_pressLowSpin, m_pressEnabledCheck,
                             0.1060, 0.0966, 0.0, 1.0, 0.001, 4, " MPa"), 1, 0);

    groupGrid->addWidget(createChannelGroup(
                             "🌿 CO₂报警", m_co2HighSpin, m_co2LowSpin, m_co2EnabledCheck,
                             1000.0, 0.0, 0.0, 5000.0, 50.0, 0, " ppm"), 1, 1);

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_confirmBtn = new QPushButton("确定", this);
    m_cancelBtn  = new QPushButton("取消", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);

    connect(m_confirmBtn, &QPushButton::clicked, this, &SettingsDialog::onConfirm);
    connect(m_cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);

    mainLayout->addLayout(groupGrid);
    mainLayout->addLayout(btnLayout);
}

AlarmConfig SettingsDialog::getTempConfig() const
{
    AlarmConfig c;
    c.highLimit = m_tempHighSpin->value();
    c.lowLimit  = m_tempLowSpin->value();
    c.enabled   = m_tempEnabledCheck->isChecked();
    return c;
}

AlarmConfig SettingsDialog::getHumConfig() const
{
    AlarmConfig c;
    c.highLimit = m_humHighSpin->value();
    c.lowLimit  = m_humLowSpin->value();
    c.enabled   = m_humEnabledCheck->isChecked();
    return c;
}

AlarmConfig SettingsDialog::getPressConfig() const
{
    AlarmConfig c;
    c.highLimit = m_pressHighSpin->value();
    c.lowLimit  = m_pressLowSpin->value();
    c.enabled   = m_pressEnabledCheck->isChecked();
    return c;
}

AlarmConfig SettingsDialog::getCo2Config() const
{
    AlarmConfig c;
    c.highLimit = m_co2HighSpin->value();
    c.lowLimit  = m_co2LowSpin->value();
    c.enabled   = m_co2EnabledCheck->isChecked();
    return c;
}

void SettingsDialog::setTempConfig(const AlarmConfig& config)
{
    m_tempHighSpin->setValue(config.highLimit);
    m_tempLowSpin->setValue(config.lowLimit);
    m_tempEnabledCheck->setChecked(config.enabled);
}

void SettingsDialog::setHumConfig(const AlarmConfig& config)
{
    m_humHighSpin->setValue(config.highLimit);
    m_humLowSpin->setValue(config.lowLimit);
    m_humEnabledCheck->setChecked(config.enabled);
}

void SettingsDialog::setPressConfig(const AlarmConfig& config)
{
    m_pressHighSpin->setValue(config.highLimit);
    m_pressLowSpin->setValue(config.lowLimit);
    m_pressEnabledCheck->setChecked(config.enabled);
}

void SettingsDialog::setCo2Config(const AlarmConfig& config)
{
    m_co2HighSpin->setValue(config.highLimit);
    m_co2LowSpin->setValue(config.lowLimit);
    m_co2EnabledCheck->setChecked(config.enabled);
}

void SettingsDialog::onConfirm()
{
    if (m_tempHighSpin->value() <= m_tempLowSpin->value()) {
        QMessageBox::warning(this, "设置错误", "温度上限必须大于下限！");
        return;
    }
    if (m_humHighSpin->value() <= m_humLowSpin->value()) {
        QMessageBox::warning(this, "设置错误", "湿度上限必须大于下限！");
        return;
    }
    if (m_pressHighSpin->value() <= m_pressLowSpin->value()) {
        QMessageBox::warning(this, "设置错误", "压力上限必须大于下限！");
        return;
    }
    accept();
}
