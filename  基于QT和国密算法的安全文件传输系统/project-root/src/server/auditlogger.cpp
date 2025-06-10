/*
以下步骤会被服务器记录到`audit.log`审计日志中：

一、 服务器生命周期
* 服务器启动时。
* 服务器启动失败时。
* 服务器成功启动并开始在端口1234监听时。

二、 客户端连接与断开
* 当一个新的客户端连接被分配给一个处理线程时。
* 当一个客户端连接关闭或断开时。

三、 文件上传过程
* 当服务器收到一个用户的[上传尝试]请求时，会记录用户的UUID、尝试上传的文件名和文件大小。
* 当收到的上传请求格式（`UPLOAD_ENC`指令）无效时。
* 当为上传请求解密SM4会话密钥[失败]时。
* 当服务器无法在本地创建文件用于写入时。
* 文件接收完毕后，哈希校验[成功]（文件完整性确认）。
* 文件接收完毕后，哈希校验[失败]（文件可能已损坏或被篡改）。

四、 文件下载过程
* 当服务器收到一个用户的[下载尝试]请求时，会记录用户的UUID和请求的文件名。
* 当用户请求下载的文件在服务器上[未找到]时。
* 当服务器成功将加密文件发送给用户时。

这些日志都由`AuditLogger`类负责写入，每条记录都会自动包含精确到秒的时间戳，以确保事件的可追溯性。
*/

#include "auditlogger.h"
#include <QCoreApplication>
#include <QDebug>

AuditLogger& AuditLogger::instance() {
    static AuditLogger logger;
    return logger;
}

/*
1. 初始化（首次访问）
该流程在服务器启动后，第一次调用 AuditLogger::instance().logEvent(...) 时触发：

	1.AuditLogger::instance() 方法被调用。
	2.方法内部的 static AuditLogger logger; 语句被执行，这会调用 AuditLogger 类的构造函数 AuditLogger::AuditLogger()，且在整个程序的生命周期中仅执行这一次。
	3.构造函数执行以下操作：
		使用 QCoreApplication::applicationDirPath() 获取程序当前可执行文件所在的路径。
		将路径与日志文件名 "audit.log" 拼接，确定日志文件的完整路径。
		尝试以只写（WriteOnly）、追加（Append）和文本（Text）模式打开这个文件。
		如果文件打开成功，就将一个 QTextStream 对象关联到这个文件，为后续的文本写入做准备。
		如果文件打开失败（例如因为权限问题），它会向控制台输出一条关键警告信息。
*/

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
