#include "auditlogger.h"
#include <QCoreApplication>
#include <QDebug>

AuditLogger& AuditLogger::instance() {
    static AuditLogger logger;
    return logger;
}

AuditLogger::AuditLogger() {
    // 日志文件将保存在可执行文件同目录下
    m_logFile.setFileName(QCoreApplication::applicationDirPath() + "/audit.log");
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
    } else {
        qWarning() << "CRITICAL: Failed to open audit.log file:" << m_logFile.fileName();
    }
}

AuditLogger::~AuditLogger() {
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void AuditLogger::logEvent(const QString& message) {
    QMutexLocker locker(&m_mutex);
    if (!m_logFile.isOpen()) {
        return;
    }
    
    // 格式: YYYY-MM-DD hh:mm:ss | 事件描述
    QString logMessage = QString("%1 | %2\n")
                             .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                             .arg(message);
    m_logStream << logMessage;
    m_logStream.flush(); // 确保日志立即写入文件
}
