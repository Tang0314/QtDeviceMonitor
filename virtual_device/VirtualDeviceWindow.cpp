#include "VirtualDeviceWindow.h"
#include "mock/MockDataFormulas.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDateTime>

VirtualDeviceWindow::VirtualDeviceWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_server(new QTcpServer(this))
    , m_sendTimer(new QTimer(this))
{
    setupUI();

    connect(m_server, &QTcpServer::newConnection,
            this, &VirtualDeviceWindow::onNewConnection);
    connect(m_sendTimer, &QTimer::timeout,
            this, &VirtualDeviceWindow::onSendTimer);
}

VirtualDeviceWindow::~VirtualDeviceWindow()
{
    if (m_server->isListening()) m_server->close();
}

void VirtualDeviceWindow::setupUI()
{
    setWindowTitle("QtDeviceMonitor - 虚拟设备");
    setMinimumSize(500, 400);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // 控制区
    QGroupBox* ctrlGroup = new QGroupBox("服务器控制", this);
    QHBoxLayout* ctrlLayout = new QHBoxLayout(ctrlGroup);

    ctrlLayout->addWidget(new QLabel("端口:", this));
    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1024, 65535);
    m_portSpin->setValue(8888);
    ctrlLayout->addWidget(m_portSpin);

    m_startStopBtn = new QPushButton("▶ 启动", this);
    m_startStopBtn->setFixedHeight(35);
    ctrlLayout->addWidget(m_startStopBtn);

    // 状态区
    QGroupBox* statusGroup = new QGroupBox("状态", this);
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    m_statusLabel = new QLabel("● 未启动", this);
    m_statusLabel->setStyleSheet("color: gray;");
    m_clientLabel = new QLabel("连接客户端: 0", this);
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_clientLabel);

    // 日志区
    QGroupBox* logGroup = new QGroupBox("发送日志", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    m_logEdit = new QTextEdit(this);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(200);
    logLayout->addWidget(m_logEdit);

    connect(m_startStopBtn, &QPushButton::clicked,
            this, &VirtualDeviceWindow::onStartStop);

    mainLayout->addWidget(ctrlGroup);
    mainLayout->addWidget(statusGroup);
    mainLayout->addWidget(logGroup);
}

void VirtualDeviceWindow::onStartStop()
{
    if (m_server->isListening()) {
        m_sendTimer->stop();
        m_server->close();
        for (auto* c : m_clients) c->disconnectFromHost();
        m_clients.clear();
        m_startStopBtn->setText("▶ 启动");
        m_statusLabel->setText("● 已停止");
        m_statusLabel->setStyleSheet("color: gray;");
        log("服务器已停止");
    } else {
        quint16 port = m_portSpin->value();
        if (!m_server->listen(QHostAddress::AnyIPv4, port)) {
            log("启动失败: " + m_server->errorString());
            return;
        }
        m_time = 0.0;
        m_sendTimer->start(100);
        m_startStopBtn->setText("⏹ 停止");
        m_statusLabel->setText(
            QString("● 运行中 - 监听 0.0.0.0:%1").arg(port));
        m_statusLabel->setStyleSheet("color: green;");
        log(QString("服务器已启动，监听端口 %1").arg(port));
    }
}

void VirtualDeviceWindow::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        m_clients.append(client);
        connect(client, &QTcpSocket::disconnected,
                this, &VirtualDeviceWindow::onClientDisconnected);
        m_clientLabel->setText(
            QString("连接客户端: %1").arg(m_clients.size()));
        log("客户端已连接: " + client->peerAddress().toString());
    }
}

void VirtualDeviceWindow::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
        m_clientLabel->setText(
            QString("连接客户端: %1").arg(m_clients.size()));
        log("客户端已断开");
    }
}

void VirtualDeviceWindow::onSendTimer()
{
    QString frame = generateFrame();
    QByteArray data = frame.toUtf8() + "\n";

    for (auto* client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
        }
    }

    // 每10帧显示一次日志（避免刷屏）
    if ((int)m_time % 10 == 0) {
        log("发送: " + frame);
    }

    m_time += 1.0;
}

QString VirtualDeviceWindow::generateFrame()
{
    return MockDataFormulas::toCsvFrame(MockDataFormulas::generate(m_time));
}

void VirtualDeviceWindow::log(const QString& msg)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logEdit->append(QString("[%1] %2").arg(time).arg(msg));
}
