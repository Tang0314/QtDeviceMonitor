#include "comm/SerialComm.h"
#include "data/DataParser.h"
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
    close();
    m_buffer.clear();
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
    m_buffer.clear();
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
    if (m_buffer.size() > MAX_BUFFER_SIZE) {
        emit frameDropped("receive buffer overflow", m_buffer.left(128));
        m_buffer.clear();
        return;
    }

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
                qWarning() << "Serial frame dropped:" << result.error << frame;
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
