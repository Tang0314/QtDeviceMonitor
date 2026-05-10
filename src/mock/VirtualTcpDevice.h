#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include <QtMath>

class VirtualTcpDevice : public QObject {
    Q_OBJECT

public:
    explicit VirtualTcpDevice(QObject* parent = nullptr);

    bool start(quint16 port = 8888);
    void stop();
    bool isRunning() const;

signals:
    void clientConnected(const QString& address);
    void clientDisconnected();
    void dataSent(const QString& frame);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onSendTimer();

private:
    QString generateFrame();

    QTcpServer*        m_server;
    QList<QTcpSocket*> m_clients;
    QTimer*            m_sendTimer;
    double             m_time = 0.0;
};
