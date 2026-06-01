#pragma once

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "data/DeviceData.h"

// 串口配置
struct SerialConfig {
    QString              portName  = "COM3";
    qint32               baudRate  = QSerialPort::Baud9600;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::Parity   parity   = QSerialPort::NoParity;
};

class SerialComm : public QObject {
    Q_OBJECT

public:
    explicit SerialComm(QObject* parent = nullptr);
    ~SerialComm();

    bool open(const SerialConfig& config);
    void close();
    bool isOpen() const;

    // 获取可用串口列表
    static QStringList availablePorts();

signals:
    void dataReceived(const DeviceData& data);
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& msg);
    void frameDropped(const QString& reason, const QByteArray& frame);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort* m_serial;
    QByteArray   m_buffer;
    static const int MAX_BUFFER_SIZE = 4096;
};
