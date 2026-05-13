#include "ui/SerialConfigDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

SerialConfigDialog::SerialConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
}

void SerialConfigDialog::setupUI()
{
    setWindowTitle("串口配置");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* group = new QGroupBox("串口参数", this);
    QGridLayout* grid = new QGridLayout(group);

    // 串口号
    m_portCombo = new QComboBox(this);
    m_refreshBtn = new QPushButton("刷新", this);
    QHBoxLayout* portLayout = new QHBoxLayout();
    portLayout->addWidget(m_portCombo);
    portLayout->addWidget(m_refreshBtn);

    // 波特率
    m_baudCombo = new QComboBox(this);
    m_baudCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    m_baudCombo->setCurrentText("9600");

    // 数据位
    m_dataBitsCombo = new QComboBox(this);
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentText("8");

    // 停止位
    m_stopBitsCombo = new QComboBox(this);
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    m_stopBitsCombo->setCurrentText("1");

    // 校验位
    m_parityCombo = new QComboBox(this);
    m_parityCombo->addItems({"无", "奇校验", "偶校验"});
    m_parityCombo->setCurrentText("无");

    grid->addWidget(new QLabel("串口号:"),  0, 0);
    grid->addLayout(portLayout,             0, 1);
    grid->addWidget(new QLabel("波特率:"),  1, 0);
    grid->addWidget(m_baudCombo,            1, 1);
    grid->addWidget(new QLabel("数据位:"),  2, 0);
    grid->addWidget(m_dataBitsCombo,        2, 1);
    grid->addWidget(new QLabel("停止位:"),  3, 0);
    grid->addWidget(m_stopBitsCombo,        3, 1);
    grid->addWidget(new QLabel("校验位:"),  4, 0);
    grid->addWidget(m_parityCombo,          4, 1);

    // 按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_confirmBtn = new QPushButton("连接", this);
    m_cancelBtn  = new QPushButton("取消", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);
    btnLayout->addWidget(m_cancelBtn);

    connect(m_refreshBtn, &QPushButton::clicked,
            this, &SerialConfigDialog::onRefreshPorts);
    connect(m_confirmBtn, &QPushButton::clicked,
            this, &SerialConfigDialog::onConfirm);
    connect(m_cancelBtn,  &QPushButton::clicked,
            this, &QDialog::reject);

    mainLayout->addWidget(group);
    mainLayout->addLayout(btnLayout);

    // 初始化串口列表
    onRefreshPorts();
}

void SerialConfigDialog::onRefreshPorts()
{
    m_portCombo->clear();
    QStringList ports = SerialComm::availablePorts();
    if (ports.isEmpty()) {
        m_portCombo->addItem("无可用串口");
    } else {
        m_portCombo->addItems(ports);
    }
}

SerialConfig SerialConfigDialog::getConfig() const
{
    SerialConfig config;
    config.portName = m_portCombo->currentText();
    config.baudRate = m_baudCombo->currentText().toInt();

    // 数据位
    int db = m_dataBitsCombo->currentText().toInt();
    switch (db) {
    case 5: config.dataBits = QSerialPort::Data5; break;
    case 6: config.dataBits = QSerialPort::Data6; break;
    case 7: config.dataBits = QSerialPort::Data7; break;
    default: config.dataBits = QSerialPort::Data8;
    }

    // 停止位
    QString sb = m_stopBitsCombo->currentText();
    if (sb == "1.5") config.stopBits = QSerialPort::OneAndHalfStop;
    else if (sb == "2") config.stopBits = QSerialPort::TwoStop;
    else config.stopBits = QSerialPort::OneStop;

    // 校验位
    int pi = m_parityCombo->currentIndex();
    if (pi == 1) config.parity = QSerialPort::OddParity;
    else if (pi == 2) config.parity = QSerialPort::EvenParity;
    else config.parity = QSerialPort::NoParity;

    return config;
}

void SerialConfigDialog::onConfirm()
{
    if (m_portCombo->currentText() == "无可用串口") {
        return;
    }
    accept();
}
