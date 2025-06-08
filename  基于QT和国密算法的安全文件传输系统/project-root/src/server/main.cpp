#include "dbmanager.h"
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QObject>
#include <QDir>
#include <QRegularExpression>

// GmSSL headers for encryption
#include <gmssl/sm2.h>
#include <gmssl/sm3.h>
#include <gmssl/sm4.h>
#include <gmssl/rand.h>
#include <gmssl/error.h>
#include "auditlogger.h"


// Helper to convert hex string to bytes
QByteArray QByteArrayFromHex(const QByteArray &hex) {
    return QByteArray::fromHex(hex);
}

// SocketHandler类：处理每个客户端连接
class SocketHandler : public QObject {
    Q_OBJECT
public:
    // 构造函数现在接收一个 socketDescriptor 而不是 QTcpSocket*
    explicit SocketHandler(qintptr socketDescriptor, QObject *parent = nullptr)
        : QObject(parent), m_socketDescriptor(socketDescriptor) {

        uploadDir = QCoreApplication::applicationDirPath() + "/uploads/";
        QDir().mkpath(uploadDir);
    }

public slots:
    // 这个槽函数将在新线程中被调用
    void handleConnection() {
        AuditLogger::instance().logEvent(QString("Thread %1: Handling connection.").arg(QString::number((quintptr)QThread::currentThreadId())));

        // 在新线程内创建和初始化socket
        socket = new QTcpSocket(this);
        if (!socket->setSocketDescriptor(m_socketDescriptor)) {
            qWarning() << "Failed to set socket descriptor in thread:" << socket->errorString();
            deleteLater();
            return;
        }

        // 连接信号和槽，这和之前一样
        connect(socket, &QTcpSocket::readyRead, this, &SocketHandler::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &SocketHandler::onDisconnected);

        // 记录新连接（现在可以包含线程ID）
        AuditLogger::instance().logEvent(QString("Connection from %1 assigned to Thread %2.")
                                             .arg(socket->peerAddress().toString())
                                             .arg(QString::number((quintptr)QThread::currentThreadId())));
    }

private slots:
    void onReadyRead() {
        if (isDownloading) return;
    
        if (receivingHeader) {
            if (socket->canReadLine()) {
                QByteArray headerLine = socket->readLine().trimmed();
                qDebug() << "Received header:" << headerLine;
                
                if (headerLine.startsWith("DOWNLOAD:")) {
                    handleDownloadRequest(headerLine);
                    return;
                }
                if (headerLine.startsWith("UPLOAD_ENC:")) {
                QString line = QString::fromUtf8(headerLine);
                // --- 这是关键的修复！确保所有 (.+) 都变成了 (.+?) ---
                QRegularExpression re("UPLOAD_ENC:(.+?):(.+?):(\\d+):(.+?):(.+?):(.+?):(.+)");
                QRegularExpressionMatch match = re.match(line);

                if (match.hasMatch()) {
                    m_userUuid = match.captured(1); // 存储UUID
                    
                    QString userUuid = match.captured(1);
                    receivedFileName = match.captured(2); // 这里现在应该能正确获取到文件名了
                    expectedFileSize = match.captured(3).toLongLong();
                    
                    QByteArray sm2_priv_key_hex = match.captured(4).toLatin1();
                    QByteArray encrypted_sm4_key_hex = match.captured(5).toLatin1();
                    QByteArray iv_hex = match.captured(6).toLatin1();
                    m_receivedHashHex = match.captured(7);
                    
                    // ★ 记录上传尝试
                    AuditLogger::instance().logEvent(QString("User [%1] attempting to UPLOAD file '%2' (%3 bytes).").arg(m_userUuid).arg(receivedFileName).arg(expectedFileSize));
                                                         
                    // --- 后续代码保持不变 ---
                    QByteArray sm2_priv_key_bytes = QByteArray::fromHex(sm2_priv_key_hex);
                    QByteArray encrypted_sm4_key_bytes = QByteArray::fromHex(encrypted_sm4_key_hex);
                    iv = QByteArray::fromHex(iv_hex);

                    SM2_KEY sm2_key;
                    sm2_key_set_private_key(&sm2_key, (const uint8_t*)sm2_priv_key_bytes.constData());

                    uint8_t decrypted_sm4_key_buf[16];
                    size_t decrypted_sm4_key_len = 0;

                    if (sm2_decrypt(&sm2_key,
                                    (const uint8_t*)encrypted_sm4_key_bytes.constData(),
                                    encrypted_sm4_key_bytes.size(),
                                    decrypted_sm4_key_buf,
                                    &decrypted_sm4_key_len) != 1) {
                        qWarning() << "SM2 key decryption failed.";
                        AuditLogger::instance().logEvent(QString("FAILURE: SM2 key decryption failed for user [%1].").arg(m_userUuid));
                        socket->close();
                        return;
                    }

                    sm4_key = QByteArray((const char*)decrypted_sm4_key_buf, decrypted_sm4_key_len);
                    sm4_set_decrypt_key(&sm4_dec_key, (const uint8_t*)sm4_key.constData());
                    
                    qInfo() << "Successfully decrypted SM4 key for file:" << receivedFileName;
                    
                    receivingHeader = false;

                    QString filePath = uploadDir + receivedFileName;
                    file.setFileName(filePath);
                    if (!file.open(QIODevice::WriteOnly)) {
                        qWarning() << "Cannot open file for writing:" << filePath;
                        AuditLogger::instance().logEvent(QString("FAILURE: Cannot open file '%1' for writing.").arg(filePath));
                        socket->close();
                        deleteLater();
                        return;
                    }
                    
                    QByteArray remainingData = socket->readAll();
                    if (!remainingData.isEmpty()) {
                        processEncryptedData(remainingData);
                    }
                } else {
                     qWarning() << "Invalid UPLOAD_ENC format:" << line;
                     AuditLogger::instance().logEvent(QString("WARNING: Received invalid UPLOAD_ENC format from %1.").arg(socket->peerAddress().toString()));
                     socket->write("ERROR: Invalid upload request format.\n");
                }
            }

            }
        } 
        else {
            // Process encrypted file chunks
            processEncryptedData(socket->readAll());
        }
    }

    void processEncryptedData(const QByteArray& encryptedData) {
        uint8_t decrypted_buf[65536 + 16]; // Buffer for decrypted data
        size_t decrypted_len = 0;

        if (sm4_cbc_padding_decrypt(&sm4_dec_key, (uint8_t*)iv.data(), (uint8_t*)encryptedData.constData(), encryptedData.size(), decrypted_buf, &decrypted_len) != 1) {
            qWarning() << "SM4 decryption failed during data transfer.";
            return;
        }

        if (decrypted_len > 0) {
            file.write((const char*)decrypted_buf, decrypted_len);
            
        }

        bytesReceived += encryptedData.size();
        qDebug() << "Received" << encryptedData.size() << "encrypted bytes, total:" << bytesReceived;
        
            if (bytesReceived >= expectedFileSize) {
            QString savedFilePath = file.fileName();
            file.close();

            // --- SM3 HASH VERIFICATION ---
            QFile receivedFile(savedFilePath);
            receivedFile.open(QIODevice::ReadOnly);
            QByteArray receivedContent = receivedFile.readAll();
            receivedFile.close();

            uint8_t computed_hash[SM3_DIGEST_SIZE];
            // 修正: 添加类型转换
            sm3_digest((const uint8_t*)receivedContent.constData(), receivedContent.size(), computed_hash);
            QString computedHashHex = QByteArray((const char*)computed_hash, sizeof(computed_hash)).toHex();
            // --- END HASH VERIFICATION ---

            if (computedHashHex == m_receivedHashHex) {
                 // ★ 记录上传成功和哈希验证成功
                 AuditLogger::instance().logEvent(QString("SUCCESS: File '%1' from user [%2] received. Hash verification PASSED.").arg(receivedFileName).arg(m_userUuid));
                qInfo() << "File received and hash verified successfully:" << receivedFileName;
            } else {
                // ★ 记录哈希验证失败
                AuditLogger::instance().logEvent(QString("FAILURE: File '%1' from user [%2] received. Hash verification FAILED.").arg(receivedFileName).arg(m_userUuid));
                qWarning() << "HASH VERIFICATION FAILED for file:" << receivedFileName;
            }

            receivingHeader = true;
            bytesReceived = 0;
        }
    }

    void handleDownloadRequest(const QByteArray &headerLine) {
        QRegularExpression re("DOWNLOAD:(.+?):(.+)");
        QRegularExpressionMatch match = re.match(QString::fromUtf8(headerLine));
    
        if (!match.hasMatch()) {
            qWarning() << "Invalid download format:" << headerLine;
            return;
        }
    
        QString userUuid = match.captured(1).trimmed();
        QString requestedFile = match.captured(2).trimmed();
        
        // ★ 记录下载尝试
        AuditLogger::instance().logEvent(QString("User [%1] attempting to DOWNLOAD file '%2'.").arg(userUuid).arg(requestedFile));

    
        QString filePath = uploadDir + requestedFile;
        QFile downloadFile(filePath);

        if (!downloadFile.exists() || !downloadFile.open(QIODevice::ReadOnly)) {
            // ★ 记录文件未找到
            AuditLogger::instance().logEvent(QString("FAILURE: File '%1' not found for user [%2].").arg(requestedFile).arg(userUuid));
            socket->write("ERROR:File not found\n");
            return;
        }
        

        
        isDownloading = true;
        // --- SM3 HASH CALCULATION (using sm3_digest) ---
        QByteArray fileContent = downloadFile.readAll();
        downloadFile.close(); // Close it, we'll send from the byte array

        uint8_t hash_output[SM3_DIGEST_SIZE];
        // 修正: 添加类型转换
        sm3_digest((const uint8_t*)fileContent.constData(), fileContent.size(), hash_output);
        // --- END HASH CALCULATION ---
        
        // 1. Generate SM2 and SM4 keys for this transfer
        SM2_KEY sm2_key;
        sm2_key_generate(&sm2_key);

        uint8_t sm4_key_raw[16];
        uint8_t iv_raw[16];
        rand_bytes(sm4_key_raw, 16);
        rand_bytes(iv_raw, 16);

        // 2. Encrypt SM4 key with SM2 public key
        uint8_t encrypted_sm4_key[128];
        size_t encrypted_sm4_key_len;
        sm2_encrypt(&sm2_key, sm4_key_raw, sizeof(sm4_key_raw), encrypted_sm4_key, &encrypted_sm4_key_len);

        // 3. Encrypt the file with SM4

        QByteArray encryptedData;
        encryptedData.resize(fileContent.size() + SM4_BLOCK_SIZE);
        size_t encrypted_len = 0;

        SM4_KEY sm4_enc_key;
        sm4_set_encrypt_key(&sm4_enc_key, sm4_key_raw);
        sm4_cbc_padding_encrypt(&sm4_enc_key, iv_raw, (uint8_t*)fileContent.data(), fileContent.size(), (uint8_t*)encryptedData.data(), &encrypted_len);
        encryptedData.resize(encrypted_len);
        
        // 4. Send header with keys and IV
        QByteArray sm2_priv_key_bytes( (char*)sm2_key.private_key, 32);
        
        QByteArray hash_hex = QByteArray((const char*)hash_output, sizeof(hash_output)).toHex();
        
        QString header = QString("FILE_ENC:%1:%2:%3:%4:%5:%6\n")

            .arg(requestedFile)
            .arg(encrypted_len) // Send encrypted size
            .arg(QString(sm2_priv_key_bytes.toHex()))
            .arg(QString(QByteArray((char*)encrypted_sm4_key, encrypted_sm4_key_len).toHex()))
            .arg(QString(QByteArray((char*)iv_raw, 16).toHex()))
            .arg(QString(hash_hex));

        socket->write(header.toUtf8());
        socket->write(encryptedData);
        
        // ★ 记录下载成功
        AuditLogger::instance().logEvent(QString("SUCCESS: File '%1' sent to user [%2].").arg(requestedFile).arg(userUuid));


        qInfo() << "Encrypted file sent:" << requestedFile;
        isDownloading = false;
    }

    void onDisconnected() {
        // ★ 记录连接断开
        AuditLogger::instance().logEvent(QString("Connection from %1 closed on Thread %2.")
                                             .arg(socket->peerAddress().toString())
                                             .arg(QString::number((quintptr)QThread::currentThreadId())));

        if (file.isOpen()) {
            file.close();
            qInfo() << "File saved as:" << file.fileName();
        }
        socket->deleteLater();
        deleteLater();
    }

private:
    QTcpSocket *socket;
    QFile file;
    QString uploadDir;
    bool receivingHeader = true;
    qint64 expectedFileSize = 0;
    qint64 bytesReceived = 0;
    QString receivedFileName;
    bool isDownloading = false;
    
    // Crypto variables
    QString m_receivedHashHex; // Use a member prefix to avoid confusion
    QByteArray sm4_key;
    QByteArray iv;
    SM4_KEY sm4_dec_key;
    QString m_userUuid; // 添加一个成员变量来存储用户UUID
    qintptr m_socketDescriptor;

};




//创建新的多线程服务器类
class ThreadedServer : public QTcpServer {
    Q_OBJECT
public:
    explicit ThreadedServer(QObject *parent = nullptr) : QTcpServer(parent) {}

protected:
    // 当有新连接时，Qt会自动调用这个函数
    void incomingConnection(qintptr socketDescriptor) override {
        qInfo() << "New connection request with descriptor:" << socketDescriptor;

        // 1. 创建一个新的线程
        QThread *thread = new QThread;

        // 2. 创建一个处理器，它将在新线程中运行
        SocketHandler *handler = new SocketHandler(socketDescriptor);
        handler->moveToThread(thread);

        // 3. 设置信号槽，确保线程和处理器能正确地启动和销毁
        //    当线程启动后，自动调用handleConnection来初始化socket
        connect(thread, &QThread::started, handler, &SocketHandler::handleConnection);
        //    当处理器对象销毁时（例如在onDisconnected中调用deleteLater），让线程退出
        connect(handler, &QObject::destroyed, thread, &QThread::quit);
        //    当线程结束后，自动删除线程对象，防止内存泄漏
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        // 4. 启动线程
        thread->start();
    }
};






int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    AuditLogger::instance().logEvent("Server starting up...");
    
    // 使用我们新的多线程服务器类
    ThreadedServer server;
    
    if (!server.listen(QHostAddress::Any, 1234)) {
        qCritical() << "Server could not start!";
        AuditLogger::instance().logEvent("CRITICAL: Server failed to start.");
        return 1;
    }
    
    qInfo() << "Threaded server listening on port 1234...";
    AuditLogger::instance().logEvent("Server started successfully. Listening on port 1234.");

    // 不再需要之前用于处理 newConnection 的 connect 语句了
    
    return app.exec();
}


#include "main.moc"
