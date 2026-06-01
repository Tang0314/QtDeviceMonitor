#pragma once

#include <QString>

// 简易日志系统 —— 将 qDebug/qWarning/qCritical 输出重定向到文件
class LogManager {
public:
    // 初始化：安装消息处理程序，日志写入 appDir/userdata/app.log
    static void install(const QString& appDir);

    // 安装默认路径（QCoreApplication::applicationDirPath() + "/userdata/app.log"）
    static void installDefault();
    static void shutdown();
};
