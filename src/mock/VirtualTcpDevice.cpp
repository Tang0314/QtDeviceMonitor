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
    double temp  = 25.0 + 5.0 * qSin(m_time * 0.1);
    double press = 1.05 + 0.1 * qSin(m_time * 0.15);

    QString status;
    if (temp > 28.0) {
        status = "ALARM";
    } else if (temp > 27.0) {
        status = "WARN";
    } else {
        status = "OK";
    }

    return QString("%1,%2,%3")
        .arg(temp,  0, 'f', 1)
        .arg(press, 0, 'f', 2)
        .arg(status);
}
