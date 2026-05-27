#pragma once

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QListWidget>
#include <QtMath>

class VirtualDeviceWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit VirtualDeviceWindow(QWidget* parent = nullptr);
    ~VirtualDeviceWindow();

private slots:
    void onStartStop();
    void onNewConnection();
    void onClientDisconnected();
    void onSendTimer();

private:
    void setupUI();
    void log(const QString& msg);
    QString generateFrame();

    // UI
    QPushButton* m_startStopBtn;
    QSpinBox*    m_portSpin;
    QLabel*      m_statusLabel;
    QLabel*      m_clientLabel;
    QTextEdit*   m_logEdit;

    // 网络
    QTcpServer*        m_server;
    QList<QTcpSocket*> m_clients;
    QTimer*            m_sendTimer;
    double             m_time = 0.0;
};
