#include "mock/VirtualTcpDevice.h"

VirtualTcpDevice::VirtualTcpDevice(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_sendTimer(new QTimer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this, &VirtualTcpDevice::onNewConnection);
    connect(m_sendTimer, &QTimer::timeout,
            this, &VirtualTcpDevice::onSendTimer);
}

bool VirtualTcpDevice::start(quint16 port)
{
    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        return false;
    }
    m_sendTimer->start(100);  // 100ms 发送一帧
    return true;
}

void VirtualTcpDevice::stop()
{
    m_sendTimer->stop();
    for (auto* client : m_clients) {
        client->disconnectFromHost();
    }
    m_clients.clear();
    m_server->close();
}

bool VirtualTcpDevice::isRunning() const
{
    return m_server->isListening();
}

void VirtualTcpDevice::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* client = m_server->nextPendingConnection();
        m_clients.append(client);

        connect(client, &QTcpSocket::disconnected,
                this, &VirtualTcpDevice::onClientDisconnected);

        emit clientConnected(client->peerAddress().toString());
    }
}

void VirtualTcpDevice::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
        emit clientDisconnected();
    }
}

void VirtualTcpDevice::onSendTimer()
{
    if (m_clients.isEmpty()) {
        m_time += 1.0;
        return;
    }

    QString frame = generateFrame();
    QByteArray data = frame.toUtf8() + "\n";

    for (auto* client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
        }
    }

    emit dataSent(frame);
    m_time += 1.0;
}

QString VirtualTcpDevice::generateFrame()
{
    // 冷库温度：-18℃ ± 3℃
    double temp = -18.0 + 3.0 * qSin(m_time * 0.05);

    // 湿度：85% ± 10%
    double humidity = 85.0 + 10.0 * qSin(m_time * 0.08 + 1.0);

    // 压力：0.1013 ± 0.005 MPa
    double pressure = 0.1013 + 0.005 * qSin(m_time * 0.03 + 2.0);

    // CO₂：800 ± 300 ppm
    double co2 = 800.0 + 300.0 * qSin(m_time * 0.06 + 0.5);

    // 门状态
    int cycle = (int)m_time % 300;
    int door = (cycle >= 200 && cycle < 230) ? 1 : 0;

    // 状态判断
    bool tempAlarm = temp > -15.0 || temp < -23.0;
    bool co2Alarm  = co2 > 1000.0;
    bool doorWarn  = door == 1;
    bool humWarn   = humidity > 95.0 || humidity < 60.0;

    QString status;
    if (tempAlarm || co2Alarm) {
        status = "ALARM";
    } else if (doorWarn || humWarn) {
        status = "WARN";
    } else {
        status = "OK";
    }

    // 格式：温度,湿度,压力,CO2,门状态,状态码
    return QString("%1,%2,%3,%4,%5,%6")
        .arg(temp,     0, 'f', 1)
        .arg(humidity, 0, 'f', 1)
        .arg(pressure, 0, 'f', 4)
        .arg(co2,      0, 'f', 0)
        .arg(door)
        .arg(status);
}
