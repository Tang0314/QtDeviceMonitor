#include "comm/TcpComm.h"
#include "data/DataParser.h"
#include <QDebug>

TcpComm::TcpComm(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setInterval(3000);  // 3秒重连

    connect(m_socket, &QTcpSocket::connected,
            this, &TcpComm::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TcpComm::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TcpComm::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred,
            this, &TcpComm::onError);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &TcpComm::onReconnectTimer);
}

TcpComm::~TcpComm()
{
    disconnectFromDevice();
}

void TcpComm::connectToDevice(const QString& host, quint16 port)
{
    m_host = host;
    m_port = port;
    m_autoReconnect = true;
    m_buffer.clear();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    m_socket->connectToHost(host, port);
}

void TcpComm::disconnectFromDevice()
{
    m_autoReconnect = false;
    m_reconnectTimer->stop();
    m_buffer.clear();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool TcpComm::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpComm::onConnected()
{
    m_reconnectTimer->stop();
    emit connectionStateChanged(true);
    qDebug() << "TCP 已连接:" << m_host << m_port;
}

void TcpComm::onDisconnected()
{
    emit connectionStateChanged(false);
    if (m_autoReconnect) {
        m_reconnectTimer->start();
    }
}

void TcpComm::onReadyRead()
{
    m_buffer += m_socket->readAll();
    if (m_buffer.size() > MAX_BUFFER_SIZE) {
        emit frameDropped("receive buffer overflow", m_buffer.left(128));
        m_buffer.clear();
        return;
    }

    // 按行解析（每帧以 \n 结尾）
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QByteArray frame = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx + 1);

        if (!frame.isEmpty()) {
            ParseResult result = DataParser::parseFrame(frame);
            if (result.isValid()) {
                emit dataReceived(result.data);
            } else {
                emit frameDropped(result.error, frame);
                qWarning() << "TCP frame dropped:" << result.error << frame;
            }
        }
    }
}

void TcpComm::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    emit errorOccurred(m_socket->errorString());
}

void TcpComm::onReconnectTimer()
{
    if (!isConnected()) {
        qDebug() << "尝试重连...";
        m_socket->connectToHost(m_host, m_port);
    }
}
