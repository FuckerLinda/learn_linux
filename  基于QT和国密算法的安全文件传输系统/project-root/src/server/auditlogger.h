#ifndef AUDITLOGGER_H
#define AUDITLOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

// 一个简单的单例日志记录器，用于审计
class AuditLogger {
public:
    static AuditLogger& instance();
    void logEvent(const QString& message);

private:
    AuditLogger();
    ~AuditLogger();
    AuditLogger(const AuditLogger&) = delete;
    AuditLogger& operator=(const AuditLogger&) = delete;

    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex; // 用于保证多线程写入安全
};

#endif // AUDITLOGGER_H
