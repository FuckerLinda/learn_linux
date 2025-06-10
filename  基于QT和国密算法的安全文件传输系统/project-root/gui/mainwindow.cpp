#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include "authwidgets.h"
#include <QThread>
#include <QProgressBar>
#include <QFileInfo>
#include <openssl/bn.h>

//====================================================================
//
//          MainWindow 类的实现
//
//====================================================================

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    dbManager(&DBManager::instance()),
    socket(new QTcpSocket(this)),
    fileSize(0),
    bytesWritten(0),
    currentUserUuid(""),
    m_headerHandlingDone(false),
    m_serverKeyReceived(false)
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    QWidget *authChoicePage = new QWidget;
    QVBoxLayout *authLayout = new QVBoxLayout(authChoicePage);
    QPushButton *loginBtn = new QPushButton("Login", this);
    QPushButton *registerBtn = new QPushButton("Register", this);
    authLayout->addWidget(loginBtn);
    authLayout->addWidget(registerBtn);
    LoginPage *loginPage = new LoginPage;
    RegisterPage *registerPage = new RegisterPage;
    QWidget *fileTransferPage = createFileTransferPage();
    stackedWidget->addWidget(authChoicePage);
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(registerPage);
    stackedWidget->addWidget(fileTransferPage);
    connect(loginBtn, &QPushButton::clicked, [this] { stackedWidget->setCurrentIndex(1); });
    connect(registerBtn, &QPushButton::clicked, [this] { stackedWidget->setCurrentIndex(2); });
    connect(loginPage, &LoginPage::loginRequested, this, &MainWindow::handleLogin);
    connect(loginPage, &LoginPage::showRegister, [this] { stackedWidget->setCurrentIndex(2); });
    connect(registerPage, &RegisterPage::registerRequested, this, &MainWindow::handleRegister);
    connect(registerPage, &RegisterPage::showLogin, [this] { stackedWidget->setCurrentIndex(1); });
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::bytesWritten, this, &MainWindow::onBytesWritten);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
}


QWidget* MainWindow::createFileTransferPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    userInfoLabel = new QLabel("User: Not logged in", page);
    selectButton = new QPushButton("Select File", page);
    sendButton = new QPushButton("Send File", page);
    sendButton->setEnabled(false);
    fileLabel = new QLabel("No file selected", page);
    progressBar = new QProgressBar(page);
    progressBar->setVisible(false);
    viewHistoryBtn = new QPushButton("View All Server Files", page);
    logoutButton = new QPushButton("Logout", page);
    layout->addWidget(userInfoLabel);
    layout->addWidget(selectButton);
    layout->addWidget(fileLabel);
    layout->addWidget(sendButton);
    layout->addWidget(progressBar);
    layout->addWidget(viewHistoryBtn);
    layout->addWidget(logoutButton);
    connect(selectButton, &QPushButton::clicked, [this]() {
        selectedFilePath = QFileDialog::getOpenFileName(this, "Select File");
        if (!selectedFilePath.isEmpty()) {
            fileLabel->setText(selectedFilePath);
            sendButton->setEnabled(true);
        }
    });
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(viewHistoryBtn, &QPushButton::clicked, this, &MainWindow::onViewHistoryClicked);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);
    return page;
}

void MainWindow::onLogoutButtonClicked() {
    currentUserUuid.clear();
    selectedFilePath.clear();
    userInfoLabel->setText("User: Not logged in");
    fileLabel->setText("No file selected");
    sendButton->setEnabled(false);
    progressBar->setVisible(false);
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->disconnectFromHost();
    }
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onViewHistoryClicked() {
    if (currentUserUuid.isEmpty()) {
        QMessageBox::warning(this, "Error", "Not logged in!");
        return;
    }
    FileHistoryDialog dialog(currentUserUuid, this);
    dialog.exec();
}

void MainWindow::updateUIForPermissions(bool isAdmin) {
    viewHistoryBtn->setVisible(true);
    selectButton->setVisible(isAdmin);
    sendButton->setVisible(isAdmin);
    if (isAdmin) {
        fileLabel->setText("No file selected");
    } else {
        fileLabel->setText("File upload is reserved for administrators.");
    }
}

void MainWindow::handleLogin(const QString &user, const QString &pass) {
    QString uuid;
    if (dbManager->loginUser(user, pass, uuid)) {
        currentUserUuid = uuid;
        userInfoLabel->setText("用户 UUID: " + uuid);
        bool isAdmin = dbManager->isAdminUser(uuid);
        updateUIForPermissions(isAdmin);
        showFileTransferPage(uuid);
        QMessageBox::information(this, "Success", "Logged in! Your UUID: " + uuid);
    } else {
        QMessageBox::warning(this, "Error", "Invalid credentials");
    }
}

void MainWindow::handleRegister(const QString &user, const QString &pass, const QString &confirm, const QString &inviteCode) {
    bool isAdmin = (inviteCode == "114514");
    if (pass != confirm) {
        QMessageBox::warning(this, "Error", "Passwords do not match");
        return;
    }
    QString uuid;
    if (dbManager->registerUser(user, pass, uuid, isAdmin)) {
        QMessageBox::information(this, "Success", "Registered! Your UUID: " + uuid);
        stackedWidget->setCurrentIndex(1);
    } else {
        QMessageBox::warning(this, "Error", "Registration failed");
    }
}

void MainWindow::showFileTransferPage(const QString &uuid) {
    stackedWidget->setCurrentIndex(3);
    if (userInfoLabel) {
        userInfoLabel->setText("用户 UUID: " + uuid);
    }
}

void MainWindow::onSendButtonClicked() {
    qDebug() << "DEBUG: onSendButtonClicked() called.";
    if (selectedFilePath.isEmpty()) { return; }
    QFile fileToHash(selectedFilePath);
    if(!fileToHash.open(QIODevice::ReadOnly)) { return; }
    QByteArray fileContent = fileToHash.readAll();
    fileToHash.close();
    
    //【加密步骤1：计算哈希】
    // 在文件发送前，使用SM3算法计算整个文件的哈希值，用于服务器端进行完整性校验。
    uint8_t hash_output[SM3_DIGEST_SIZE];
    sm3_digest((const uint8_t*)fileContent.constData(), fileContent.size(), hash_output);
    m_uploadFileHash = QByteArray((const char*)hash_output, SM3_DIGEST_SIZE);
    qDebug() << "DEBUG: File hash calculated:" << m_uploadFileHash.toHex();
    fileToSend.setFileName(selectedFilePath);
    if (!fileToSend.open(QIODevice::ReadOnly)) { return; }
    fileSize = fileToSend.size();
    bytesWritten = 0;
    m_headerHandlingDone = false;
    progressBar->setValue(0);
    progressBar->setVisible(true);
    m_serverKeyReceived = false;
    qDebug() << "DEBUG: Connecting to host...";
    socket->connectToHost("localhost", 1234);
}

void MainWindow::onConnected() {
    qDebug() << "DEBUG: onConnected() called. Connection established. Waiting for server public key...";
}

void MainWindow::onReadyRead() {
    qDebug() << "DEBUG: onReadyRead() called.";
    if (m_serverKeyReceived) {
        qDebug() << "DEBUG: Server key already received, ignoring.";
        return;
    }
    if (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        qDebug() << "DEBUG: Received line from server:" << line;
        if (line.startsWith("PUBKEY:")) {
        
            //【加密步骤2：接收并解析服务器公钥】
            // 从服务器发来的"PUBKEY:"消息中，提取出公钥的十六进制字符串。
            QByteArray pub_key_hex = line.mid(7);
            QByteArray pub_key_bytes = QByteArray::fromHex(pub_key_hex);
            qDebug() << "DEBUG: Received public key hex:" << pub_key_hex;
            SM2_POINT public_point;
            if (sm2_point_from_octets(&public_point, (const uint8_t*)pub_key_bytes.constData(), pub_key_bytes.size()) != 1) {
                QMessageBox::critical(this, "Error", "Failed to parse server public key bytes.");
                socket->disconnectFromHost();
                return;
            }
            SM2_KEY temp_key;
            if (sm2_key_generate(&temp_key) != 1) {
                 QMessageBox::critical(this, "Error", "Failed to init crypto context.");
                 return;
            }
            if (sm2_key_set_public_key(&temp_key, &public_point) != 1) {
                 QMessageBox::critical(this, "Error", "Failed to set server public key.");
                 return;
            }
            m_server_sm2_key = temp_key;
            m_serverKeyReceived = true;
            qDebug() << "DEBUG: Server public key received and set successfully. Proceeding to send upload request.";
            sendUploadRequest();
        }
    }
}


void MainWindow::sendUploadRequest() {
    qDebug() << "DEBUG: sendUploadRequest() called.";
    if (!m_serverKeyReceived) { return; }
    
    //【加密步骤3：生成临时对称密钥】
    // 使用rand_bytes为本次文件传输生成一个一次性的SM4会话密钥和初始化向量(IV)。
    rand_bytes(sm4_key, 16);
    rand_bytes(iv, 16);
    qDebug() << "DEBUG: Generated temporary SM4 key and IV.";
    
    //【加密步骤4：使用服务器公钥加密会话密钥】
    // 调用sm2_encrypt，使用刚刚从服务器收到的公钥(m_server_sm2_key)来加密SM4会话密钥。
    uint8_t encrypted_sm4_key[256];
    size_t encrypted_sm4_key_len;
    if (sm2_encrypt(&m_server_sm2_key, sm4_key, sizeof(sm4_key), encrypted_sm4_key, &encrypted_sm4_key_len) != 1) {
        QMessageBox::critical(this, "Error", "Failed to encrypt session key with server's public key.");
        socket->disconnectFromHost();
        return;
    }
    qDebug() << "DEBUG: SM4 key encrypted with server's public key.";
    
    //【加密步骤5：构造并发送协议头】
    // 构造不包含任何私钥的、新的安全协议头，并将其发送给服务器。
    QByteArray encrypted_sm4_key_hex = QByteArray((const char*)encrypted_sm4_key, encrypted_sm4_key_len).toHex();
    QByteArray iv_hex = QByteArray((const char*)iv, sizeof(iv)).toHex();
    QByteArray hash_hex = m_uploadFileHash.toHex();
    QFileInfo fileInfo(selectedFilePath);
    QString header = QString("UPLOAD_ENC:%1:%2:%3:%4:%5:%6\n")
                         .arg(currentUserUuid)
                         .arg(fileInfo.fileName())
                         .arg(fileSize)
                         .arg(QString(encrypted_sm4_key_hex))
                         .arg(QString(iv_hex))
                         .arg(QString(hash_hex));
    qDebug() << "DEBUG: Sending new protocol header:" << header;
    socket->write(header.toUtf8());
    m_headerHandlingDone = true;
    onBytesWritten(0);
}

void MainWindow::onBytesWritten(qint64 bytes) {
    if (!m_headerHandlingDone) { return; }
    bytesWritten += bytes;
    qDebug() << "DEBUG_CLIENT: Bytes written signal received, value:" << bytes << ", Total written:" << bytesWritten;
    if (fileSize > 0) {
        progressBar->setValue(static_cast<int>((bytesWritten * 100) / fileSize));
    }
    if (bytesWritten >= fileSize) {
        qDebug() << "DEBUG_CLIENT: File sending complete.";
        dbManager->recordFileTransfer(currentUserUuid, QFileInfo(selectedFilePath).fileName(), fileSize);
        fileToSend.close();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->disconnectFromHost();
        }
        QMessageBox::information(this, "Success", "File sent successfully!");
        progressBar->setVisible(false);
        m_headerHandlingDone = false;
    } else {
        qDebug() << "DEBUG_CLIENT: Reading next chunk from file.";
        QByteArray plain_chunk = fileToSend.read(65536);
        if (!plain_chunk.isEmpty()) {
            //【加密步骤6：使用对称密钥加密文件内容】
            // 使用临时的SM4密钥和IV，通过sm4_cbc_padding_encrypt函数加密文件块。
            uint8_t encrypted_chunk[65536 + 16];
            size_t encrypted_len;
            SM4_KEY sm4_enc_key_struct;
            sm4_set_encrypt_key(&sm4_enc_key_struct, sm4_key);
            if (sm4_cbc_padding_encrypt(&sm4_enc_key_struct, iv, (const uint8_t*)plain_chunk.constData(), plain_chunk.size(), encrypted_chunk, &encrypted_len) != 1) {
                QMessageBox::critical(this, "Error", "File encryption failed during transfer.");
                socket->disconnectFromHost();
                return;
            }
            qDebug() << "DEBUG_CLIENT: Writing encrypted chunk of size" << encrypted_len;
            socket->write((const char*)encrypted_chunk, encrypted_len);
        }
    }
}

void MainWindow::onErrorOccurred(QAbstractSocket::SocketError error) {
    QMessageBox::critical(this, "Error", "Connection error: " + socket->errorString());
}


//====================================================================
//
/*          ★ FileHistoryDialog 类的实现 

FileHistoryDialog是一个独立的对话框窗口 。作用：

展示文件列表：当用户点击主界面上的“查看文件”按钮时，这个对话框会弹出。它会从数据库中获取服务器上所有可供下载的文件，并将它们以列表的形式展示给用户 。
管理文件下载：它为用户提供了“下载”按钮。用户在列表中选择一个文件后，点击此按钮即可启动整个文件下载流程。
封装下载逻辑：它内部封装了所有与文件下载相关的功能，包括创建新的网络连接、向服务器发送下载请求、接收加密数据、解密文件、保存到本地以及校验文件完整性等全部步骤 。
*/
//====================================================================

FileHistoryDialog::~FileHistoryDialog() {
    if (downloadSocket) {
        downloadSocket->disconnectFromHost();
        if (downloadSocket->state() != QAbstractSocket::UnconnectedState) {
            downloadSocket->waitForDisconnected();
        }
        delete downloadSocket;
    }
    if (downloadFile) {
        if (downloadFile->isOpen()) {
            downloadFile->close();
        }
        delete downloadFile;
    }
}


FileHistoryDialog::FileHistoryDialog(const QString &userUuid, QWidget *parent)
    : QDialog(parent), currentUserUuid(userUuid),
      downloadSocket(nullptr), downloadFile(nullptr),
      fileSize(0), bytesReceived(0),
      downloadProgress(new QProgressBar(this)),
      headerReceived(false) {
    setWindowTitle("All Server Files");
    setFixedSize(600, 400);
    QVBoxLayout *layout = new QVBoxLayout(this);
    fileList = new QListWidget(this);
    layout->addWidget(fileList);
    DBManager *dbManager = &DBManager::instance();
    QVector<QString> history = dbManager->getAllFileHistory();
    for (const QString &record : history) {
        fileList->addItem(record);
    }
    downloadProgress->setVisible(false);
    layout->addWidget(downloadProgress);
    QPushButton *downloadBtn = new QPushButton("Download Selected File", this);
    layout->addWidget(downloadBtn);
    connect(downloadBtn, &QPushButton::clicked, this, &FileHistoryDialog::onDownloadClicked);
}

//当用户点击“下载”按钮时被触发，是整个下载流程的起点。
void FileHistoryDialog::onDownloadClicked() {
    if (downloadSocket && downloadSocket->isOpen()) {
        downloadSocket->disconnectFromHost();
        downloadSocket->deleteLater();
    }
    if (downloadFile && downloadFile->isOpen()) {
        downloadFile->close();
        delete downloadFile;
    }
    
    QListWidgetItem *item = fileList->currentItem();
    if (!item) return;

    QStringList parts = item->text().split("|");
    if (parts.size() < 2) return;
    currentDownloadFileName = parts[1].trimmed();

    QString savePath = QFileDialog::getSaveFileName(this, "Save File", currentDownloadFileName);
    if (savePath.isEmpty()) return;

    downloadFile = new QFile(savePath, this);
    if (!downloadFile->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file for writing: " + downloadFile->errorString());
        delete downloadFile; downloadFile = nullptr;
        return;
    }

    //【解密步骤1：生成临时的SM2密钥对】
    // 在发起下载请求前，客户端为自己生成一对SM2密钥，用于接收服务器加密后的会话密钥。
    if (sm2_key_generate(&m_download_sm2_key) != 1) {
        QMessageBox::critical(this, "Crypto Error", "Failed to generate download key pair.");
        return;
    }

    downloadSocket = new QTcpSocket(this);
    connect(downloadSocket, &QTcpSocket::connected, this, &FileHistoryDialog::onDownloadConnected);
    connect(downloadSocket, &QTcpSocket::readyRead, this, &FileHistoryDialog::onDownloadReadyRead);
    connect(downloadSocket, &QTcpSocket::disconnected, this, &FileHistoryDialog::onDownloadDisconnected);
    connect(downloadSocket, &QTcpSocket::errorOccurred, this, &FileHistoryDialog::onDownloadError);

    fileSize = 0;
    bytesReceived = 0;
    headerReceived = false;
    downloadProgress->setVisible(true);
    downloadProgress->setValue(0);
    downloadProgress->setMinimum(0);
    downloadProgress->setMaximum(100);
    downloadSocket->connectToHost("localhost", 1234);
}

//当downloadSocket成功连接到服务器后被自动调用
void FileHistoryDialog::onDownloadConnected() {
    //【解密步骤2：发送包含公钥的下载请求】
    // 将上一步生成的临时公钥，随下载请求一同发给服务器，以便服务器用它来加密。
    uint8_t pub_key_buf[65];
    sm2_point_to_uncompressed_octets(&m_download_sm2_key.public_key, pub_key_buf);
    QByteArray pub_key_hex = QByteArray((const char*)pub_key_buf, sizeof(pub_key_buf)).toHex();

    QString request = QString("DOWNLOAD:%1:%2:%3\n")
                          .arg(currentUserUuid)
                          .arg(currentDownloadFileName)
                          .arg(QString(pub_key_hex)); // 带上自己的公钥

    downloadSocket->write(request.toUtf8());
}

//当服务器有数据发送过来时被触发，是处理下载数据的核心
void FileHistoryDialog::onDownloadReadyRead() {
    if (!headerReceived) {
        if (downloadSocket->canReadLine()) {
            QByteArray headerLine = downloadSocket->readLine();
            if (headerLine.startsWith("FILE_ENC:")) {
                // ... (解析服务器发来的FILE_ENC头部) ...
                QRegularExpression re("FILE_ENC:(.+):(\\d+):(.+):(.+):(.+)");
                QRegularExpressionMatch match = re.match(QString::fromUtf8(headerLine));
                if (!match.hasMatch()) {
                    qWarning() << "Invalid FILE_ENC header format:" << headerLine;
                    return;
                }

                currentDownloadFileName = match.captured(1);
                fileSize = match.captured(2).toLongLong();
                QByteArray encrypted_sm4_key_hex = match.captured(3).toLatin1();
                QByteArray iv_hex = match.captured(4).toLatin1();
                m_receivedHashHex = match.captured(5);

                //【解密步骤3：使用自己的私钥解密会话密钥】
                // 使用之前为本次下载生成的私钥(m_download_sm2_key)，通过sm2_decrypt解密出会话密钥。
                uint8_t sm4_key_raw[16];
                size_t sm4_key_len;
                if (sm2_decrypt(&m_download_sm2_key, (const uint8_t*)QByteArray::fromHex(encrypted_sm4_key_hex).constData(), QByteArray::fromHex(encrypted_sm4_key_hex).size(), sm4_key_raw, &sm4_key_len) != 1) {
                    qWarning() << "Failed to decrypt SM4 key for download.";
                    return;
                }
                sm4_set_decrypt_key(&sm4_dec_key, sm4_key_raw);
                iv = QByteArray::fromHex(iv_hex);

                headerReceived = true;
                downloadProgress->setMaximum(fileSize);
                processEncryptedFileData(downloadSocket->readAll());
            } else if (headerLine.startsWith("ERROR:")) {
                QMessageBox::critical(this, "Server Error", QString::fromUtf8(headerLine));
                downloadSocket->disconnectFromHost();
            }
        }
    } else {
        processEncryptedFileData(downloadSocket->readAll());
    }
}

//负责解密文件内容
void FileHistoryDialog::processEncryptedFileData(const QByteArray& data) {
    if (data.isEmpty() || !downloadFile || !downloadFile->isOpen()) return;
    bytesReceived += data.size();
    uint8_t decrypted_buf[65536 + 16];
    size_t decrypted_len;
    
    //【解密步骤4：使用会话密钥解密文件内容】
    // 使用刚刚解密出的SM4密钥，通过sm4_cbc_padding_decrypt解密文件数据块。
    if (sm4_cbc_padding_decrypt(&sm4_dec_key, (uint8_t*)iv.constData(), (uint8_t*)data.constData(), data.size(), decrypted_buf, &decrypted_len) != 1) {
        qWarning() << "SM4 decryption failed during download.";
    } else {
        downloadFile->write((char*)decrypted_buf, decrypted_len);
        downloadProgress->setValue(bytesReceived);
    }
    if (bytesReceived >= fileSize) {
        completeDownload();
    }
}

//当所有文件数据都接收完毕后被调用，进行收尾工作
void FileHistoryDialog::completeDownload() {
    downloadFile->close();
    downloadProgress->setValue(downloadProgress->maximum());
    QFile downloadedFile(downloadFile->fileName());
    if (!downloadedFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not re-open downloaded file for verification.");
        return;
    }
    QByteArray downloadedContent = downloadedFile.readAll();
    downloadedFile.close();
    uint8_t computed_hash[SM3_DIGEST_SIZE];
    
    //【解密步骤5：哈希校验】
    // 文件接收完毕后，计算下载好的文件的SM3哈希，并与服务器发来的原始哈希比对。
    sm3_digest((const uint8_t*)downloadedContent.constData(), downloadedContent.size(), computed_hash);
    QString computedHashHex = QByteArray((const char*)computed_hash, sizeof(computed_hash)).toHex();
    QString message = QString("File '%1' downloaded successfully!").arg(currentDownloadFileName);
    if (computedHashHex == m_receivedHashHex) {
        message += "\n\nHash verification successful. File integrity is confirmed.";
        QMessageBox::information(this, "Success", message);
    } else {
        message += "\n\nERROR: Hash verification failed! The file may be corrupted.";
        QMessageBox::critical(this, "Verification Failed", message);
    }
    downloadSocket->disconnectFromHost();
}

//处理下载连接正常断开的情况
void FileHistoryDialog::onDownloadDisconnected() {
    if (downloadFile && downloadFile->isOpen()) {
        downloadFile->close();
    }
    if (downloadSocket) {
        downloadSocket->deleteLater();
        downloadSocket = nullptr;
    }
    downloadProgress->setVisible(false);
}


void FileHistoryDialog::onDownloadError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    QMessageBox::critical(this, "Error", "Download error: " + downloadSocket->errorString());
    downloadSocket->disconnectFromHost();
}
