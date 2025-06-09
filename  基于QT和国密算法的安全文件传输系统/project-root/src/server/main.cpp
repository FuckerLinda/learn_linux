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

#include <gmssl/sm2.h>
#include <gmssl/sm3.h>
#include <gmssl/sm4.h>
#include <gmssl/rand.h>
#include <gmssl/error.h>
#include "auditlogger.h"

// 1. 定义全局密钥和初始化函数
SM2_KEY g_server_sm2_key;

void initialize_server_keys() {
    if (sm2_key_generate(&g_server_sm2_key) != 1) {
        qCritical() << "FATAL: Could not generate server's long-term SM2 key.";
        exit(1);
    }
    qInfo() << "Server's long-term SM2 key has been generated for this session.";
}

QByteArray QByteArrayFromHex(const QByteArray &hex) {
    return QByteArray::fromHex(hex);
}

// 2. SocketHandler类能够接收和存储服务器密钥
class SocketHandler : public QObject {
    Q_OBJECT
public:
    explicit SocketHandler(qintptr socketDescriptor, const SM2_KEY* serverKey, QObject *parent = nullptr)
        : QObject(parent), m_socketDescriptor(socketDescriptor), m_serverKey(serverKey) {
        uploadDir = QCoreApplication::applicationDirPath() + "/uploads/";
        QDir().mkpath(uploadDir);
    }

public slots:
    void handleConnection() {
        AuditLogger::instance().logEvent(QString("Thread %1: Handling connection.").arg(QString::number((quintptr)QThread::currentThreadId())));
        socket = new QTcpSocket(this);
        if (!socket->setSocketDescriptor(m_socketDescriptor)) {
            qWarning() << "Failed to set socket descriptor in thread:" << socket->errorString();
            deleteLater();
            return;
        }
        connect(socket, &QTcpSocket::readyRead, this, &SocketHandler::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &SocketHandler::onDisconnected);
        AuditLogger::instance().logEvent(QString("Connection from %1 assigned to Thread %2.")
                                             .arg(socket->peerAddress().toString())
                                             .arg(QString::number((quintptr)QThread::currentThreadId())));

        // 发送服务器的公钥给客户端
        uint8_t pub_key_buf[65];
        sm2_point_to_uncompressed_octets(&m_serverKey->public_key, pub_key_buf);
        QByteArray pub_key_hex = QByteArray((const char*)pub_key_buf, sizeof(pub_key_buf)).toHex();
        QByteArray header = "PUBKEY:" + pub_key_hex + "\n";
        socket->write(header);
        socket->flush();
        AuditLogger::instance().logEvent("Sent public key to client " + socket->peerAddress().toString());
    }

private slots:
    void onReadyRead() {
        if (isDownloading) return;

        if (receivingHeader) {
            if (socket->canReadLine()) {
                QByteArray headerLine = socket->readLine().trimmed();
                qDebug() << "DEBUG_SERVER: Received line:" << headerLine; // 调试日志

                if (headerLine.startsWith("DOWNLOAD:")) {
                    handleDownloadRequest(headerLine);
                    return;
                }
                if (headerLine.startsWith("UPLOAD_ENC:")) {
                    QString line = QString::fromUtf8(headerLine);
                    QRegularExpression re("UPLOAD_ENC:(.+?):(.+?):(\\d+):(.+?):(.+?):(.+)");
                    QRegularExpressionMatch match = re.match(line);
                    if (match.hasMatch()) {
                        m_userUuid = match.captured(1);
                        receivedFileName = match.captured(2);
                        expectedFileSize = match.captured(3).toLongLong();
                        QByteArray encrypted_sm4_key_hex = match.captured(4).toLatin1();
                        QByteArray iv_hex = match.captured(5).toLatin1();
                        m_receivedHashHex = match.captured(6);
                        AuditLogger::instance().logEvent(QString("User [%1] attempting to UPLOAD file '%2' (%3 bytes).").arg(m_userUuid).arg(receivedFileName).arg(expectedFileSize));
                        QByteArray encrypted_sm4_key_bytes = QByteArray::fromHex(encrypted_sm4_key_hex);
                        iv = QByteArray::fromHex(iv_hex);
                        uint8_t decrypted_sm4_key_buf[16];
                        size_t decrypted_sm4_key_len = 0;
                        if (sm2_decrypt(m_serverKey,
                                        (const uint8_t*)encrypted_sm4_key_bytes.constData(),
                                        encrypted_sm4_key_bytes.size(),
                                        decrypted_sm4_key_buf,
                                        &decrypted_sm4_key_len) != 1) {
                            qWarning() << "SM2 key decryption failed. The client might be using a wrong public key.";
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
                            socket->close();
                            return;
                        }
                        QByteArray remainingData = socket->readAll();
                        qDebug() << "DEBUG_SERVER: Data received immediately after header, size:" << remainingData.size();
                        if (!remainingData.isEmpty()) {
                            processEncryptedData(remainingData);
                        }
                    } else {
                         qWarning() << "Invalid UPLOAD_ENC format:" << line;
                         socket->write("ERROR: Invalid upload request format.\n");
                    }
                }
            }
        }
        else {
            qDebug() << "DEBUG_SERVER: receivingHeader is false, reading more data...";
            processEncryptedData(socket->readAll());
        }
    }

    void processEncryptedData(const QByteArray& encryptedData) {
        qDebug() << "DEBUG_SERVER: processEncryptedData called with size:" << encryptedData.size();
        
        uint8_t decrypted_buf[65536 + 16];
        size_t decrypted_len = 0;
        if (sm4_cbc_padding_decrypt(&sm4_dec_key, (uint8_t*)iv.data(), (uint8_t*)encryptedData.constData(), encryptedData.size(), decrypted_buf, &decrypted_len) != 1) {
            qWarning() << "SM4 decryption failed during data transfer.";
            return;
        }

        if (decrypted_len > 0) {
            file.write((const char*)decrypted_buf, decrypted_len);
        }

        bytesReceived += encryptedData.size();
        qDebug() << "DEBUG_SERVER: Total bytes received (encrypted):" << bytesReceived << ", Expected file size (plaintext):" << expectedFileSize;

        if (bytesReceived >= expectedFileSize) {
            qDebug() << "DEBUG_SERVER: File receiving complete. Closing file and starting verification...";
            
            QString savedFilePath = file.fileName();
            file.close();

            QFile receivedFile(savedFilePath);
            if (!receivedFile.open(QIODevice::ReadOnly)) {
                 qWarning() << "Could not re-open file for verification:" << savedFilePath;
                 return;
            }
            QByteArray receivedContent = receivedFile.readAll();
            receivedFile.close();
            
            qDebug() << "DEBUG_SERVER: Re-opened and read file for hashing, size:" << receivedContent.size();

            uint8_t computed_hash[SM3_DIGEST_SIZE];
            sm3_digest((const uint8_t*)receivedContent.constData(), receivedContent.size(), computed_hash);
            QString computedHashHex = QByteArray((const char*)computed_hash, sizeof(computed_hash)).toHex();

            qDebug() << "DEBUG_SERVER: Received Hash:" << m_receivedHashHex;
            qDebug() << "DEBUG_SERVER: Computed Hash:" << computedHashHex;

            if (computedHashHex == m_receivedHashHex) {
                 AuditLogger::instance().logEvent(QString("SUCCESS: File '%1' from user [%2] received. Hash verification PASSED.").arg(receivedFileName).arg(m_userUuid));
                qInfo() << "File received and hash verified successfully:" << receivedFileName;
            } else {
                AuditLogger::instance().logEvent(QString("FAILURE: File '%1' from user [%2] received. Hash verification FAILED.").arg(receivedFileName).arg(m_userUuid));
                qWarning() << "HASH VERIFICATION FAILED for file:" << receivedFileName;
            }
            receivingHeader = true;
            bytesReceived = 0;
        }
    }

    // 在 src/server/main.cpp 的 SocketHandler 类中

void handleDownloadRequest(const QByteArray &headerLine) {
    // 1. ★ 新的正则表达式，用于解析客户端发来的公钥
    QRegularExpression re("DOWNLOAD:(.+?):(.+?):(.+)");
    QRegularExpressionMatch match = re.match(QString::fromUtf8(headerLine));

    if (!match.hasMatch()) {
        qWarning() << "Invalid (new) download format:" << headerLine;
        return;
    }

    QString userUuid = match.captured(1).trimmed();
    QString requestedFile = match.captured(2).trimmed();
    QByteArray client_pub_key_hex = match.captured(3).trimmed().toLatin1(); // 客户端的临时公钥

    AuditLogger::instance().logEvent(QString("User [%1] attempting to DOWNLOAD file '%2'.").arg(userUuid).arg(requestedFile));

    QString filePath = uploadDir + requestedFile;
    QFile downloadFile(filePath);

    if (!downloadFile.exists() || !downloadFile.open(QIODevice::ReadOnly)) {
        AuditLogger::instance().logEvent(QString("FAILURE: File '%1' not found for user [%2].").arg(requestedFile).arg(userUuid));
        socket->write("ERROR:File not found\n");
        return;
    }

    isDownloading = true;
    QByteArray fileContent = downloadFile.readAll();
    downloadFile.close();

    uint8_t hash_output[SM3_DIGEST_SIZE];
    sm3_digest((const uint8_t*)fileContent.constData(), fileContent.size(), hash_output);

    // 2. ★ 将客户端发来的公钥字节，加载到一个SM2_KEY对象中用于加密
    SM2_POINT client_public_point;
    QByteArray client_pub_key_bytes = QByteArray::fromHex(client_pub_key_hex);
    if (sm2_point_from_octets(&client_public_point, (const uint8_t*)client_pub_key_bytes.constData(), client_pub_key_bytes.size()) != 1) {
        qWarning() << "Failed to parse client public key for download.";
        socket->close();
        return;
    }
    SM2_KEY client_key;
    sm2_key_generate(&client_key); // 初始化上下文
    sm2_key_set_public_key(&client_key, &client_public_point);

    // 3. ★ 生成临时的SM4密钥，并用客户端的公钥加密
    uint8_t sm4_key_raw[16], iv_raw[16];
    rand_bytes(sm4_key_raw, 16);
    rand_bytes(iv_raw, 16);

    uint8_t encrypted_sm4_key[128];
    size_t encrypted_sm4_key_len;
    sm2_encrypt(&client_key, sm4_key_raw, sizeof(sm4_key_raw), encrypted_sm4_key, &encrypted_sm4_key_len);

    // 4. ★ 用SM4密钥加密文件
    QByteArray encryptedData;
    encryptedData.resize(fileContent.size() + SM4_BLOCK_SIZE);
    size_t encrypted_len = 0;
    SM4_KEY sm4_enc_key;
    sm4_set_encrypt_key(&sm4_enc_key, sm4_key_raw);
    sm4_cbc_padding_encrypt(&sm4_enc_key, iv_raw, (uint8_t*)fileContent.data(), fileContent.size(), (uint8_t*)encryptedData.data(), &encrypted_len);
    encryptedData.resize(encrypted_len);

    // 5. ★ 发送不含任何私钥的、新的协议头
    QByteArray hash_hex = QByteArray((const char*)hash_output, sizeof(hash_output)).toHex();
    QString header = QString("FILE_ENC:%1:%2:%3:%4:%5\n") // 只有5个字段
        .arg(requestedFile)
        .arg(encrypted_len)
        .arg(QString(QByteArray((char*)encrypted_sm4_key, encrypted_sm4_key_len).toHex()))
        .arg(QString(QByteArray((char*)iv_raw, 16).toHex()))
        .arg(QString(hash_hex));

    socket->write(header.toUtf8());
    socket->write(encryptedData);

    AuditLogger::instance().logEvent(QString("SUCCESS: File '%1' sent to user [%2].").arg(requestedFile).arg(userUuid));
    qInfo() << "Encrypted file sent:" << requestedFile;
    isDownloading = false;
}
    void onDisconnected() {
        AuditLogger::instance().logEvent(QString("Connection from %1 closed on Thread %2.").arg(socket->peerAddress().toString()).arg(QString::number((quintptr)QThread::currentThreadId())));
        if (file.isOpen()) {
            file.close();
        }
        socket->deleteLater();
        deleteLater();
    }

private:
    QTcpSocket *socket;
    QFile file;
    QString uploadDir;
    bool receivingHeader = true;
    qint64 expectedFileSize = 0, bytesReceived = 0;
    QString receivedFileName;
    bool isDownloading = false;
    QString m_receivedHashHex, m_userUuid;
    QByteArray sm4_key, iv;
    SM4_KEY sm4_dec_key;
    qintptr m_socketDescriptor;
    const SM2_KEY* m_serverKey;
};


class ThreadedServer : public QTcpServer {
    Q_OBJECT
public:
    explicit ThreadedServer(QObject *parent = nullptr) : QTcpServer(parent) {}
protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QThread *thread = new QThread;
        SocketHandler *handler = new SocketHandler(socketDescriptor, &g_server_sm2_key, nullptr);
        handler->moveToThread(thread);
        connect(thread, &QThread::started, handler, &SocketHandler::handleConnection);
        connect(handler, &QObject::destroyed, thread, &QThread::quit);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
    }
};


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    initialize_server_keys();
    AuditLogger::instance().logEvent("Server starting up...");
    ThreadedServer server;
    if (!server.listen(QHostAddress::Any, 1234)) {
        qCritical() << "Server could not start!";
        return 1;
    }
    qInfo() << "Threaded server listening on port 1234...";
    return app.exec();
}

#include "main.moc"
