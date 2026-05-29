#include "config/ConfigManager.h"
#include <QCoreApplication>

ConfigManager::ConfigManager()
    : m_settings(
        QCoreApplication::applicationDirPath() + "/userdata/config.ini",
        QSettings::IniFormat)
{}

void ConfigManager::saveConfig(const QString& group, const AlarmConfig& config)
{
    m_settings.beginGroup(group);
    m_settings.setValue("highLimit", config.highLimit);
    m_settings.setValue("lowLimit",  config.lowLimit);
    m_settings.setValue("enabled",   config.enabled);
    m_settings.endGroup();
}

AlarmConfig ConfigManager::loadConfig(
    const QString& group,
    double defaultHigh,
    double defaultLow)
{
    AlarmConfig config;
    m_settings.beginGroup(group);
    config.highLimit = m_settings.value("highLimit", defaultHigh).toDouble();
    config.lowLimit  = m_settings.value("lowLimit",  defaultLow).toDouble();
    config.enabled   = m_settings.value("enabled",   true).toBool();
    m_settings.endGroup();
    return config;
}

void ConfigManager::saveTempConfig(const AlarmConfig& config)
{
    saveConfig("alarm/temperature", config);
}

void ConfigManager::saveHumConfig(const AlarmConfig& config)
{
    saveConfig("alarm/humidity", config);
}

void ConfigManager::savePressConfig(const AlarmConfig& config)
{
    saveConfig("alarm/pressure", config);
}

void ConfigManager::saveCo2Config(const AlarmConfig& config)
{
    saveConfig("alarm/co2", config);
}

AlarmConfig ConfigManager::loadTempConfig()
{
    return loadConfig("alarm/temperature", AlarmDefaults::TEMP_HIGH, AlarmDefaults::TEMP_LOW);
}

AlarmConfig ConfigManager::loadHumConfig()
{
    return loadConfig("alarm/humidity", AlarmDefaults::HUM_HIGH, AlarmDefaults::HUM_LOW);
}

AlarmConfig ConfigManager::loadPressConfig()
{
    return loadConfig("alarm/pressure", AlarmDefaults::PRESS_HIGH, AlarmDefaults::PRESS_LOW);
}

AlarmConfig ConfigManager::loadCo2Config()
{
    return loadConfig("alarm/co2", AlarmDefaults::CO2_HIGH, AlarmDefaults::CO2_LOW);
}

void ConfigManager::saveTcpHost(const QString& host)
{
    m_settings.setValue("tcp/host", host);
}

void ConfigManager::saveTcpPort(quint16 port)
{
    m_settings.setValue("tcp/port", port);
}

QString ConfigManager::loadTcpHost() const
{
    return m_settings.value("tcp/host", "127.0.0.1").toString();
}

quint16 ConfigManager::loadTcpPort() const
{
    return m_settings.value("tcp/port", 8888).toUInt();
}
