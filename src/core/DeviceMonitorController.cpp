#include "core/DeviceMonitorController.h"

#include "comm/TcpComm.h"
#include "data/DatabaseManager.h"
#include "mock/MockDataGenerator.h"
#include "mock/VirtualTcpDevice.h"

#include <QDebug>

QString dataSourceName(DataSource source)
{
    switch (source) {
    case DataSource::Mock:
        return "Mock";
    case DataSource::TCP:
        return "TCP";
    case DataSource::Serial:
        return "串口";
    case DataSource::None:
        return "空闲";
    }
    return "未知";
}

DeviceMonitorController::DeviceMonitorController(QObject* parent)
    : QObject(parent)
    , m_mockGenerator(new MockDataGenerator(this))
    , m_virtualDevice(new VirtualTcpDevice(this))
    , m_tcpComm(new TcpComm(this))
    , m_serialComm(new SerialComm(this))
    , m_alarmChecker(new AlarmChecker(this))
    , m_dbManager(new DatabaseManager(this))
{
    loadAlarmConfigs();
    applyAlarmConfigs();

    connect(m_mockGenerator, &MockDataGenerator::dataGenerated,
            this, &DeviceMonitorController::handleData);
    connect(m_tcpComm, &TcpComm::dataReceived,
            this, &DeviceMonitorController::handleData);
    connect(m_serialComm, &SerialComm::dataReceived,
            this, &DeviceMonitorController::handleData);

    connect(m_tcpComm, &TcpComm::connectionStateChanged,
            this, &DeviceMonitorController::handleTcpStateChanged);
    connect(m_tcpComm, &TcpComm::errorOccurred,
            this, &DeviceMonitorController::handleTcpError);
    connect(m_serialComm, &SerialComm::connectionStateChanged,
            this, &DeviceMonitorController::handleSerialStateChanged);
    connect(m_serialComm, &SerialComm::errorOccurred,
            this, &DeviceMonitorController::handleSerialError);

    connect(m_alarmChecker, &AlarmChecker::alarmTriggered,
            this, &DeviceMonitorController::alarmTriggered);
    connect(m_alarmChecker, &AlarmChecker::alarmCleared,
            this, &DeviceMonitorController::alarmCleared);
    connect(m_alarmChecker, &AlarmChecker::alarmTriggered,
            m_dbManager, &DatabaseManager::saveAlarm);
}

DeviceMonitorController::~DeviceMonitorController()
{
    stop();
    m_dbManager->flush();
}

bool DeviceMonitorController::initialize(const QString& dbPath)
{
    bool ok = m_dbManager->initialize(dbPath);
    if (!ok) {
        emit databaseError("数据库初始化失败: " + dbPath);
    }
    return ok;
}

DataSource DeviceMonitorController::dataSource() const
{
    return m_dataSource;
}

int DeviceMonitorController::recordCount() const
{
    return m_recordCount;
}

QDateTime DeviceMonitorController::collectStartTime() const
{
    return m_collectStartTime;
}

DatabaseManager* DeviceMonitorController::databaseManager() const
{
    return m_dbManager;
}

AlarmConfig DeviceMonitorController::tempConfig() const
{
    return m_tempConfig;
}

AlarmConfig DeviceMonitorController::humConfig() const
{
    return m_humConfig;
}

AlarmConfig DeviceMonitorController::pressConfig() const
{
    return m_pressConfig;
}

AlarmConfig DeviceMonitorController::co2Config() const
{
    return m_co2Config;
}

QString DeviceMonitorController::tcpHost() const
{
    return m_configManager.loadTcpHost();
}

quint16 DeviceMonitorController::tcpPort() const
{
    return m_configManager.loadTcpPort();
}

void DeviceMonitorController::startMock(int intervalMs)
{
    setDataSource(DataSource::Mock);
    resetCollectionStats();
    m_mockGenerator->start(intervalMs);
    emit connectionStatusChanged("● Mock 模式", "blue");
}

void DeviceMonitorController::startTcp(const QString& host, quint16 port, bool useBuiltInVirtualDevice)
{
    setDataSource(DataSource::TCP);
    resetCollectionStats();
    m_configManager.saveTcpHost(host);
    m_configManager.saveTcpPort(port);

    if (useBuiltInVirtualDevice && !m_virtualDevice->isRunning()) {
        if (!m_virtualDevice->start(port)) {
            qWarning() << "Virtual TCP device start failed on port" << port;
        }
    }

    m_tcpComm->connectToDevice(host, port);
    emit connectionStatusChanged(
        QString("● TCP 连接中 %1:%2...").arg(host).arg(port),
        "orange");
}

bool DeviceMonitorController::startSerial(const SerialConfig& config)
{
    stopCurrentSource();
    m_dataSource = DataSource::None;

    if (!m_serialComm->open(config)) {
        emit dataSourceChanged(m_dataSource);
        emit connectionStatusChanged("● 串口连接失败", "red");
        return false;
    }

    m_dataSource = DataSource::Serial;
    resetCollectionStats();
    emit dataSourceChanged(m_dataSource);
    emit connectionStatusChanged("● 已连接串口 " + config.portName, "green");
    return true;
}

void DeviceMonitorController::stop()
{
    setDataSource(DataSource::None);
    emit connectionStatusChanged("● 未连接", "gray");
}

void DeviceMonitorController::updateAlarmConfigs(
    const AlarmConfig& tempConfig,
    const AlarmConfig& humConfig,
    const AlarmConfig& pressConfig,
    const AlarmConfig& co2Config)
{
    m_tempConfig = tempConfig;
    m_humConfig = humConfig;
    m_pressConfig = pressConfig;
    m_co2Config = co2Config;

    applyAlarmConfigs();

    m_configManager.saveTempConfig(m_tempConfig);
    m_configManager.saveHumConfig(m_humConfig);
    m_configManager.savePressConfig(m_pressConfig);
    m_configManager.saveCo2Config(m_co2Config);
}

void DeviceMonitorController::handleData(const DeviceData& data)
{
    if (m_dataSource == DataSource::None || !data.isValid) {
        return;
    }

    ++m_recordCount;
    emit recordCountChanged(m_recordCount);
    emit dataReceived(data);
    m_alarmChecker->checkData(data);
    m_dbManager->saveData(data);
}

void DeviceMonitorController::handleTcpStateChanged(bool connected)
{
    if (m_dataSource != DataSource::TCP) {
        return;
    }

    if (connected) {
        emit connectionStatusChanged(
            QString("● 已连接 TCP %1:%2").arg(tcpHost()).arg(tcpPort()),
            "green");
    } else {
        emit connectionStatusChanged("● TCP 断开，重连中...", "orange");
    }
}

void DeviceMonitorController::handleTcpError(const QString& msg)
{
    emit communicationError(msg);
    if (m_dataSource == DataSource::TCP) {
        emit connectionStatusChanged("● TCP 错误: " + msg, "red");
    }
}

void DeviceMonitorController::handleSerialStateChanged(bool connected)
{
    if (m_dataSource != DataSource::Serial) {
        return;
    }

    if (connected) {
        emit connectionStatusChanged("● 已连接串口", "green");
    } else {
        emit connectionStatusChanged("● 串口已断开", "gray");
        setDataSource(DataSource::None);
    }
}

void DeviceMonitorController::handleSerialError(const QString& msg)
{
    emit communicationError(msg);
    if (m_dataSource == DataSource::Serial) {
        emit connectionStatusChanged("● 串口错误: " + msg, "red");
    }
}

void DeviceMonitorController::loadAlarmConfigs()
{
    m_tempConfig = m_configManager.loadTempConfig();
    m_humConfig = m_configManager.loadHumConfig();
    m_pressConfig = m_configManager.loadPressConfig();
    m_co2Config = m_configManager.loadCo2Config();
}

void DeviceMonitorController::applyAlarmConfigs()
{
    m_alarmChecker->setTempConfig(m_tempConfig);
    m_alarmChecker->setHumConfig(m_humConfig);
    m_alarmChecker->setPressConfig(m_pressConfig);
    m_alarmChecker->setCo2Config(m_co2Config);
}

void DeviceMonitorController::setDataSource(DataSource source)
{
    if (m_dataSource == source) {
        return;
    }

    stopCurrentSource();
    m_dataSource = source;
    emit dataSourceChanged(m_dataSource);

    if (source == DataSource::None) {
        m_collectStartTime = QDateTime();
    }
}

void DeviceMonitorController::stopCurrentSource()
{
    DataSource oldSource = m_dataSource;
    m_dataSource = DataSource::None;

    switch (oldSource) {
    case DataSource::Mock:
        m_mockGenerator->stop();
        break;
    case DataSource::TCP:
        m_tcpComm->disconnectFromDevice();
        m_virtualDevice->stop();
        break;
    case DataSource::Serial:
        m_serialComm->close();
        break;
    case DataSource::None:
        break;
    }
}

void DeviceMonitorController::resetCollectionStats()
{
    m_recordCount = 0;
    m_collectStartTime = QDateTime::currentDateTime();
    emit recordCountChanged(m_recordCount);
}
