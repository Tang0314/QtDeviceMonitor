#pragma once

#include "alarm/AlarmChecker.h"
#include "comm/SerialComm.h"
#include "config/ConfigManager.h"
#include "data/DeviceData.h"

#include <QDateTime>
#include <QObject>
#include <QProcess>
#include <QString>

class DatabaseManager;
class MockDataGenerator;
class TcpComm;
class VirtualTcpDevice;

enum class DataSource {
    None,
    Mock,
    TCP,
    Serial
};

Q_DECLARE_METATYPE(DataSource)

QString dataSourceName(DataSource source);

class DeviceMonitorController : public QObject {
    Q_OBJECT

public:
    explicit DeviceMonitorController(QObject* parent = nullptr);
    ~DeviceMonitorController();

    bool initialize(const QString& dbPath);

    DataSource dataSource() const;
    int recordCount() const;
    QDateTime collectStartTime() const;

    DatabaseManager* databaseManager() const;

    AlarmConfig tempConfig() const;
    AlarmConfig humConfig() const;
    AlarmConfig pressConfig() const;
    AlarmConfig co2Config() const;

    QString tcpHost() const;
    quint16 tcpPort() const;
    bool isVirtualSerialDeviceRunning() const;

public slots:
    void startMock(int intervalMs = 100);
    void startTcp(const QString& host, quint16 port, bool useBuiltInVirtualDevice = true);
    bool startSerial(const SerialConfig& config);
    void startVirtualSerialDevice(const QString& port = "COM5",
                                  int baudRate = 9600,
                                  int intervalMs = 100);
    void stopVirtualSerialDevice();
    void stop();
    void updateAlarmConfigs(const AlarmConfig& tempConfig,
                            const AlarmConfig& humConfig,
                            const AlarmConfig& pressConfig,
                            const AlarmConfig& co2Config);

signals:
    void dataReceived(const DeviceData& data);
    void alarmTriggered(const AlarmEvent& event);
    void alarmCleared(const QString& channel);
    void dataSourceChanged(DataSource source);
    void connectionStatusChanged(const QString& text, const QString& colorName);
    void communicationError(const QString& message);
    void databaseError(const QString& message);
    void recordCountChanged(int count);
    void virtualSerialDeviceStateChanged(bool running, const QString& message);

private slots:
    void handleData(const DeviceData& data);
    void handleTcpStateChanged(bool connected);
    void handleTcpError(const QString& msg);
    void handleSerialStateChanged(bool connected);
    void handleSerialError(const QString& msg);
    void handleVirtualSerialStarted();
    void handleVirtualSerialFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleVirtualSerialError(QProcess::ProcessError error);
    void handleVirtualSerialOutput();

private:
    void loadAlarmConfigs();
    void applyAlarmConfigs();
    void setDataSource(DataSource source);
    void stopCurrentSource();
    void resetCollectionStats();
    QString findPythonExecutable() const;
    QString virtualSerialScriptPath() const;

    MockDataGenerator* m_mockGenerator;
    VirtualTcpDevice*  m_virtualDevice;
    TcpComm*           m_tcpComm;
    SerialComm*        m_serialComm;
    AlarmChecker*      m_alarmChecker;
    DatabaseManager*   m_dbManager;
    QProcess*          m_virtualSerialProcess;
    bool               m_virtualSerialStopRequested = false;
    ConfigManager      m_configManager;

    AlarmConfig m_tempConfig;
    AlarmConfig m_humConfig;
    AlarmConfig m_pressConfig;
    AlarmConfig m_co2Config;

    DataSource m_dataSource = DataSource::None;
    int        m_recordCount = 0;
    QDateTime  m_collectStartTime;
};
