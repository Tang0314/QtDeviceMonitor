#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "data/DeviceData.h"

class TcpComm : public QObject {
    Q_OBJECT

public:
    explicit TcpComm(QObject* parent = nullptr);
    ~TcpComm();

    void connectToDevice(const QString& host, quint16 port);
    void disconnectFromDevice();
    bool isConnected() const;

signals:
    void dataReceived(const DeviceData& data);     // 解析完成的数据
    void connectionStateChanged(bool connected);   // 连接状态变化
    void errorOccurred(const QString& msg);        // 错误信息
    void frameDropped(const QString& reason, const QByteArray& frame);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();

private:
    QTcpSocket* m_socket;
    QTimer*     m_reconnectTimer;
    QByteArray  m_buffer;          // 粘包处理缓冲区

    QString  m_host;
    quint16  m_port = 8888;
    bool     m_autoReconnect = true;
    static const int MAX_BUFFER_SIZE = 4096;
};
