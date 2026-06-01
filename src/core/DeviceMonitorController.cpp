#include "core/DeviceMonitorController.h"

#include "comm/TcpComm.h"
#include "data/DatabaseManager.h"
#include "mock/MockDataGenerator.h"
#include "mock/VirtualTcpDevice.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

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
    , m_virtualSerialProcess(new QProcess(this))
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

    m_virtualSerialProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_virtualSerialProcess, &QProcess::started,
            this, &DeviceMonitorController::handleVirtualSerialStarted);
    connect(m_virtualSerialProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DeviceMonitorController::handleVirtualSerialFinished);
    connect(m_virtualSerialProcess, &QProcess::errorOccurred,
            this, &DeviceMonitorController::handleVirtualSerialError);
    connect(m_virtualSerialProcess, &QProcess::readyReadStandardOutput,
            this, &DeviceMonitorController::handleVirtualSerialOutput);
}

DeviceMonitorController::~DeviceMonitorController()
{
    stopVirtualSerialDevice();
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

bool DeviceMonitorController::isVirtualSerialDeviceRunning() const
{
    return m_virtualSerialProcess->state() != QProcess::NotRunning;
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

void DeviceMonitorController::startVirtualSerialDevice(
    const QString& port,
    int baudRate,
    int intervalMs)
{
    if (isVirtualSerialDeviceRunning()) {
        emit virtualSerialDeviceStateChanged(true, "串口模拟器已在运行");
        return;
    }

    const QString python = findPythonExecutable();
    if (python.isEmpty()) {
        emit virtualSerialDeviceStateChanged(false, "未找到 Python，请先安装 Python 或将 py/python 加入 PATH");
        return;
    }

    const QString scriptPath = virtualSerialScriptPath();
    if (!QFileInfo::exists(scriptPath)) {
        emit virtualSerialDeviceStateChanged(false, "未找到脚本: " + scriptPath);
        return;
    }

    QStringList args;
    args << scriptPath
         << "--port" << port
         << "--baud" << QString::number(baudRate)
         << "--interval-ms" << QString::number(intervalMs)
         << "--log-every" << "10";

    m_virtualSerialProcess->setProgram(python);
    m_virtualSerialProcess->setArguments(args);
    m_virtualSerialProcess->setWorkingDirectory(QFileInfo(scriptPath).absolutePath());
    m_virtualSerialStopRequested = false;
    m_virtualSerialProcess->start();
    emit virtualSerialDeviceStateChanged(true, "串口模拟器启动中...");
}

void DeviceMonitorController::stopVirtualSerialDevice()
{
    if (!isVirtualSerialDeviceRunning()) {
        emit virtualSerialDeviceStateChanged(false, "串口模拟器未运行");
        return;
    }

    m_virtualSerialStopRequested = true;
    m_virtualSerialProcess->terminate();
    if (!m_virtualSerialProcess->waitForFinished(2000)) {
        m_virtualSerialProcess->kill();
        m_virtualSerialProcess->waitForFinished(1000);
    }
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

void DeviceMonitorController::handleVirtualSerialStarted()
{
    emit virtualSerialDeviceStateChanged(true, "串口模拟器运行中");
}

void DeviceMonitorController::handleVirtualSerialFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
    const bool stopRequested = m_virtualSerialStopRequested;
    m_virtualSerialStopRequested = false;

    if (stopRequested || (exitStatus == QProcess::NormalExit && exitCode == 0)) {
        emit virtualSerialDeviceStateChanged(false, "串口模拟器已停止");
    } else {
    emit virtualSerialDeviceStateChanged(
            false,
            QString("串口模拟器异常退出，exit=%1；COM5 可能已被占用，应用请连接配对端 COM6")
                .arg(exitCode));
    }
}

void DeviceMonitorController::handleVirtualSerialError(QProcess::ProcessError error)
{
    QString message;
    switch (error) {
    case QProcess::FailedToStart:
        message = "串口模拟器启动失败";
        break;
    case QProcess::Crashed:
        message = "串口模拟器进程崩溃";
        break;
    case QProcess::Timedout:
        message = "串口模拟器进程超时";
        break;
    case QProcess::WriteError:
        message = "串口模拟器写入失败";
        break;
    case QProcess::ReadError:
        message = "串口模拟器读取失败";
        break;
    case QProcess::UnknownError:
        message = "串口模拟器未知错误";
        break;
    }
    emit virtualSerialDeviceStateChanged(isVirtualSerialDeviceRunning(), message);
}

void DeviceMonitorController::handleVirtualSerialOutput()
{
    const QString output = QString::fromLocal8Bit(m_virtualSerialProcess->readAllStandardOutput()).trimmed();
    if (!output.isEmpty()) {
        qInfo().noquote() << "[virtual_serial_device]" << output;
        if (output.contains("serial error", Qt::CaseInsensitive)
            || output.contains("PermissionError", Qt::CaseInsensitive)
            || output.contains(QRegularExpression("Access.*denied", QRegularExpression::CaseInsensitiveOption))) {
            emit virtualSerialDeviceStateChanged(
                false,
                "串口模拟器无法打开 COM5；请先断开占用 COM5 的连接，应用应连接 COM6");
        }
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

QString DeviceMonitorController::findPythonExecutable() const
{
    const QStringList candidates = {
        "py",
        "python",
        "python3"
    };

    for (const QString& candidate : candidates) {
        const QString executable = QStandardPaths::findExecutable(candidate);
        if (!executable.isEmpty()) {
            return executable;
        }
    }
    return QString();
}

QString DeviceMonitorController::virtualSerialScriptPath() const
{
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QString deployedPath = appDir.filePath("docs/virtual_serial_device.py");
    if (QFileInfo::exists(deployedPath)) {
        return deployedPath;
    }

    QDir sourceDir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 6; ++i) {
        const QString candidate = sourceDir.filePath("docs/virtual_serial_device.py");
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
        if (!sourceDir.cdUp()) {
            break;
        }
    }

    return QDir::current().filePath("docs/virtual_serial_device.py");
}
