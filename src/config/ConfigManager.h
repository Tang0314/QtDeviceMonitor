#pragma once
#include <QSettings>
#include "alarm/AlarmChecker.h"

class ConfigManager {
public:
    ConfigManager();

    void saveTempConfig(const AlarmConfig& config);
    void saveHumConfig(const AlarmConfig& config);
    void savePressConfig(const AlarmConfig& config);
    void saveCo2Config(const AlarmConfig& config);

    AlarmConfig loadTempConfig();
    AlarmConfig loadHumConfig();
    AlarmConfig loadPressConfig();
    AlarmConfig loadCo2Config();

    void    saveTcpHost(const QString& host);
    void    saveTcpPort(quint16 port);
    QString loadTcpHost() const;
    quint16 loadTcpPort() const;

private:
    AlarmConfig loadConfig(const QString& group,
                           double defaultHigh,
                           double defaultLow);
    void saveConfig(const QString& group,
                    const AlarmConfig& config);

    QSettings m_settings;
};
