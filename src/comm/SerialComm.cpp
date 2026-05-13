#include "comm/SerialComm.h"
#include <QDebug>

SerialComm::SerialComm(QObject* parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead,
            this, &SerialComm::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred,
            this, &SerialComm::onError);
}

SerialComm::~SerialComm()
{
    close();
}

bool SerialComm::open(const SerialConfig& config)
{
    m_serial->setPortName(config.portName);
    m_serial->setBaudRate(config.baudRate);
    m_serial->setDataBits(config.dataBits);
    m_serial->setStopBits(config.stopBits);
    m_serial->setParity(config.parity);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serial->errorString());
        return false;
    }

    m_buffer.clear();
    emit connectionStateChanged(true);
    qDebug() << "串口已打开:" << config.portName;
    return true;
}

void SerialComm::close()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        emit connectionStateChanged(false);
    }
}

bool SerialComm::isOpen() const
{
    return m_serial->isOpen();
}

QStringList SerialComm::availablePorts()
{
    QStringList ports;
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        ports << info.portName();
    }
    return ports;
}

void SerialComm::onReadyRead()
{
    m_buffer += m_serial->readAll();

    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QByteArray frame = m_buffer.left(idx).trimmed();
        m_buffer = m_buffer.mid(idx + 1);

        if (!frame.isEmpty()) {
            DeviceData data = parseFrame(frame);
            if (data.isValid) {
                emit dataReceived(data);
            }
        }
    }
}

void SerialComm::onError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) return;
    emit errorOccurred(m_serial->errorString());
    if (error == QSerialPort::ResourceError) {
        close();
    }
}

DeviceData SerialComm::parseFrame(const QByteArray& frame)
{
    DeviceData data;
    // 格式：温度,湿度,压力,CO2,门状态,状态码
    QList<QByteArray> parts = frame.split(',');
    if (parts.size() < 6) return data;

    bool ok1, ok2, ok3, ok4, ok5;
    double temp     = parts[0].trimmed().toDouble(&ok1);
    double humidity = parts[1].trimmed().toDouble(&ok2);
    double pressure = parts[2].trimmed().toDouble(&ok3);
    double co2      = parts[3].trimmed().toDouble(&ok4);
    int    door     = parts[4].trimmed().toInt(&ok5);

    if (!ok1 || !ok2 || !ok3 || !ok4 || !ok5) return data;

    data.timestamp   = QDateTime::currentDateTime();
    data.temperature = temp;
    data.humidity    = humidity;
    data.pressure    = pressure;
    data.co2         = co2;
    data.doorOpen    = (door == 1);
    data.statusCode  = QString::fromUtf8(parts[5].trimmed());
    data.isValid     = true;

    return data;
}
