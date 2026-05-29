#include "utils/LogManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

static QFile*    g_logFile   = nullptr;
static QTextStream* g_logStream = nullptr;
static QMutex    g_logMutex;

static QString levelToString(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:    return "DEBUG";
    case QtInfoMsg:     return "INFO";
    case QtWarningMsg:  return "WARN";
    case QtCriticalMsg: return "ERROR";
    case QtFatalMsg:    return "FATAL";
    }
    return "?????";
}

static void logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context)

    QMutexLocker locker(&g_logMutex);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString line = QString("[%1] [%2] %3\n")
                       .arg(timestamp)
                       .arg(levelToString(type))
                       .arg(msg);

    // 写文件
    if (g_logStream && g_logFile && g_logFile->isOpen()) {
        *g_logStream << line;
        g_logStream->flush();
    }

    // 同时输出到原始控制台
    fprintf(stderr, "%s", qPrintable(line));
}

void LogManager::install(const QString& appDir)
{
    QDir().mkpath(appDir + "/userdata");

    QString logPath = appDir + "/userdata/app.log";

    g_logFile = new QFile(logPath);
    g_logFile->open(QIODevice::Append | QIODevice::Text);
    g_logStream = new QTextStream(g_logFile);

    qInstallMessageHandler(logMessageHandler);

    qInfo() << "=== QtDeviceMonitor 启动 ===";
}

void LogManager::installDefault()
{
    install(QCoreApplication::applicationDirPath());
}
