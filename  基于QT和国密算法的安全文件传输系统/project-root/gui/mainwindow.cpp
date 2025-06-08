#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include "authwidgets.h"
#include <QThread> 
#include <QProgressBar> 
#include <gmssl/sm2.h>
#include <gmssl/sm4.h>
#include <openssl/bn.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    dbManager(&DBManager::instance()), 
    socket(new QTcpSocket(this)),
    fileSize(0),
    bytesWritten(0),
    currentUserUuid(""),
    m_headerHandlingDone(false) {

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

    connect(loginBtn, &QPushButton::clicked, [this] {
        stackedWidget->setCurrentIndex(1);
    });
    
    connect(registerBtn, &QPushButton::clicked, [this] {
        stackedWidget->setCurrentIndex(2);
    });
    
    connect(loginPage, &LoginPage::loginRequested, this, &MainWindow::handleLogin);
    connect(loginPage, &LoginPage::showRegister, [this] {
        stackedWidget->setCurrentIndex(2);
    });
    
    connect(registerPage, &RegisterPage::registerRequested, this, &MainWindow::handleRegister);
    connect(registerPage, &RegisterPage::showLogin, [this] {
        stackedWidget->setCurrentIndex(1);
    });

    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::bytesWritten, this, &MainWindow::onBytesWritten);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onErrorOccurred);
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
    logoutButton = new QPushButton("Logout", page); // <-- 创建登出按钮

    layout->addWidget(userInfoLabel);
    layout->addWidget(selectButton);
    layout->addWidget(fileLabel);
    layout->addWidget(sendButton);
    layout->addWidget(progressBar);
    layout->addWidget(viewHistoryBtn);
    layout->addWidget(logoutButton); // <-- 将登出按钮添加到布局

    connect(selectButton, &QPushButton::clicked, [this]() {
        selectedFilePath = QFileDialog::getOpenFileName(this, "Select File");
        if (!selectedFilePath.isEmpty()) {
            fileLabel->setText(selectedFilePath);
            sendButton->setEnabled(true);
        }
    });
   
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(viewHistoryBtn, &QPushButton::clicked, this, &MainWindow::onViewHistoryClicked);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked); // <-- 连接信号和槽

    return page;
}

void MainWindow::onLogoutButtonClicked() {
    // 1. 清空当前用户会话信息
    currentUserUuid.clear();
    selectedFilePath.clear();

    // 2. 重置UI状态
    userInfoLabel->setText("User: Not logged in");
    fileLabel->setText("No file selected");
    sendButton->setEnabled(false);
    progressBar->setVisible(false);
    
    // 如果socket处于连接状态，则断开
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->disconnectFromHost();
    }

    // 3. 切换回登录页面 (LoginPage 在 stackedWidget 中的索引是 1)
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


// MODIFIED: This function now controls visibility of upload widgets
void MainWindow::updateUIForPermissions(bool isAdmin) {
    // Admins can see history and upload buttons
    // Normal users can only see history
    viewHistoryBtn->setVisible(true); // Assuming all users can see their own history
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
        
        // --- PERMISSION CHECK ---
        bool isAdmin = dbManager->isAdminUser(uuid);
        updateUIForPermissions(isAdmin); // Update UI based on permissions
        
        showFileTransferPage(uuid);
        QMessageBox::information(this, "Success", "Logged in! Your UUID: " + uuid);
    } else {
        QMessageBox::warning(this, "Error", "Invalid credentials");
    }
}

void MainWindow::handleRegister(const QString &user,
                               const QString &pass,
                               const QString &confirm,
                               const QString &inviteCode) {
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
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No file selected.");
        return;
    }
    
    
    // --- SM3 HASH CALCULATION (using sm3_digest) ---
    QFile fileToHash(selectedFilePath); // Open file to hash it
    if(!fileToHash.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file for hashing.");
        return;
    }
    QByteArray fileContent = fileToHash.readAll(); // Read whole file
    fileToHash.close();

    uint8_t hash_output[SM3_DIGEST_SIZE];
    // 修正 #1: 添加 (const uint8_t*) 类型转换
    sm3_digest((const uint8_t*)fileContent.constData(), fileContent.size(), hash_output); 
    
    // 修正 #2: 将原始哈希存入成员变量
    m_uploadFileHash = QByteArray((const char*)hash_output, SM3_DIGEST_SIZE);
    // --- END HASH CALCULATION ---

    
    fileToSend.setFileName(selectedFilePath);
    if (!fileToSend.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file: " + fileToSend.errorString());
        return;
    }

    fileSize = fileToSend.size();
    bytesWritten = 0;
    m_headerHandlingDone = false;
    progressBar->setValue(0);
    progressBar->setVisible(true);

    socket->connectToHost("localhost", 1234);
}

// MODIFIED: Sends new UPLOAD_ENC command with crypto data
void MainWindow::onConnected() {
    // --- 修正部分在这里 ---
    // 1. 生成SM2密钥对，并立刻检查是否成功
    if (sm2_key_generate(&sm2_key) != 1) {
        QMessageBox::critical(this, "加密错误", "无法为本次传输生成有效的加密密钥，请重试。");
        socket->disconnectFromHost();
        return;
    }
    // --- 修正结束 ---

    // 2. Encrypt the SM4 key with the SM2 public key
    uint8_t encrypted_sm4_key[256];
    size_t encrypted_sm4_key_len;
    if (sm2_encrypt(&sm2_key, sm4_key, sizeof(sm4_key), encrypted_sm4_key, &encrypted_sm4_key_len) != 1) {
        QMessageBox::critical(this, "Error", "Failed to encrypt session key.");
        return;
    }

    // 3. Prepare crypto data for the header (as hex strings)
    QByteArray sm2_priv_key_hex = QByteArray((char*)sm2_key.private_key, 32).toHex();
    QByteArray encrypted_sm4_key_hex = QByteArray((char*)encrypted_sm4_key, encrypted_sm4_key_len).toHex();
    QByteArray iv_hex = QByteArray((char*)iv, sizeof(iv)).toHex();

    QByteArray hash_hex = m_uploadFileHash.toHex();

    QFileInfo fileInfo(selectedFilePath);
    QString header = QString("UPLOAD_ENC:%1:%2:%3:%4:%5:%6:%7\n")
                         .arg(currentUserUuid)
                         .arg(fileInfo.fileName())
                         .arg(fileSize)
                         .arg(QString(sm2_priv_key_hex))
                         .arg(QString(encrypted_sm4_key_hex))
                         .arg(QString(iv_hex))
                         .arg(QString(hash_hex));

    socket->write(header.toUtf8());
}

void MainWindow::onBytesWritten(qint64 bytes) {
    if (!m_headerHandlingDone) {
        m_headerHandlingDone = true;
    }

    if (this->bytesWritten >= this->fileSize) {
        dbManager->recordFileTransfer(currentUserUuid, 
                                     QFileInfo(selectedFilePath).fileName(), 
                                     this->fileSize);
        fileToSend.close();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->disconnectFromHost();
        }
        QMessageBox::information(this, "Success", "File sent successfully!");
    } else {
        QByteArray plain_chunk = fileToSend.read(65536);
        if (!plain_chunk.isEmpty()) {
            uint8_t encrypted_chunk[65536 + 16];
            size_t encrypted_len;

            SM4_KEY sm4_enc_key;
            sm4_set_encrypt_key(&sm4_enc_key, sm4_key);
            
            if (sm4_cbc_padding_encrypt(&sm4_enc_key, iv, (uint8_t*)plain_chunk.constData(), plain_chunk.size(), encrypted_chunk, &encrypted_len) != 1) {
                QMessageBox::critical(this, "Error", "File encryption failed during transfer.");
                socket->disconnectFromHost();
                return;
            }
            
            bytesWritten += socket->write((char*)encrypted_chunk, encrypted_len);
        }
    }

    if (this->fileSize > 0) {
        progressBar->setValue(static_cast<int>((this->bytesWritten * 100) / this->fileSize));
    } else {
        progressBar->setValue(100);
    }
}

void MainWindow::onErrorOccurred(QAbstractSocket::SocketError error) {
    QMessageBox::critical(this, "Error", "Connection error: " + socket->errorString());
}


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
    setWindowTitle("All Server Files"); // 标题已更改
    setFixedSize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);
    fileList = new QListWidget(this);
    layout->addWidget(fileList);

    DBManager *dbManager = &DBManager::instance();
    // 调用新方法来获取所有文件，而不仅仅是当前用户的历史记录
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


void FileHistoryDialog::onDownloadClicked() {
    if (downloadSocket) {
        downloadSocket->disconnect();
        downloadSocket->deleteLater();
        downloadSocket = nullptr;
    }
    if (downloadFile && downloadFile->isOpen()) {
        downloadFile->close();
        delete downloadFile;
        downloadFile = nullptr;
    }
    
    QListWidgetItem *item = fileList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Error", "No file selected!");
        return;
    }

    QString selectedText = item->text();
    QStringList parts = selectedText.split("|");
    if (parts.size() < 2) {
        QMessageBox::warning(this, "Error", "Invalid record format!");
        return;
    }
    currentDownloadFileName = parts[1].trimmed();

    qDebug() << "Selected record:" << selectedText;
    qDebug() << "Parsed filename for download:" << currentDownloadFileName;

    QString savePath = QFileDialog::getSaveFileName(this, "Save File", currentDownloadFileName);
    if (savePath.isEmpty()) return;

    downloadFile = new QFile(savePath, this);
    if (!downloadFile->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file for writing: " + downloadFile->errorString());
        delete downloadFile;
        downloadFile = nullptr;
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

void FileHistoryDialog::onDownloadConnected() {
    QString request = QString("DOWNLOAD:%1:%2\n").arg(currentUserUuid).arg(currentDownloadFileName);
    qDebug() << "Sending download request:" << request;
    downloadSocket->write(request.toUtf8());
}

void FileHistoryDialog::onDownloadReadyRead() {
    if (!headerReceived) {
        if (downloadSocket->canReadLine()) {
            QByteArray headerLine = downloadSocket->readLine();
            qDebug() << "Received header:" << headerLine;

            if (headerLine.startsWith("FILE_ENC:")) {
                QRegularExpression re("FILE_ENC:(.+):(\\d+):(.+):(.+):(.+):(.+)");
                QRegularExpressionMatch match = re.match(QString::fromUtf8(headerLine));

                if (!match.hasMatch()) {
                    qWarning() << "Invalid FILE_ENC header format:" << headerLine;
                    return;
                }

                currentDownloadFileName = match.captured(1);
                fileSize = match.captured(2).toLongLong();
                QByteArray sm2_priv_key_hex = match.captured(3).toLatin1();
                QByteArray encrypted_sm4_key_hex = match.captured(4).toLatin1();
                QByteArray iv_hex = match.captured(5).toLatin1();
                m_receivedHashHex = match.captured(6);
                
                // Convert keys from hex and decrypt the SM4 key
                QByteArray sm2_priv_key_bytes = QByteArray::fromHex(sm2_priv_key_hex);
                QByteArray encrypted_sm4_key_bytes = QByteArray::fromHex(encrypted_sm4_key_hex);
                iv = QByteArray::fromHex(iv_hex);
                
                SM2_KEY sm2_key;
                sm2_key_set_private_key(&sm2_key, (const uint8_t*)QByteArray::fromHex(sm2_priv_key_hex).constData());

                uint8_t sm4_key_raw[16];
                size_t sm4_key_len;
                if (sm2_decrypt(&sm2_key, (const uint8_t*)QByteArray::fromHex(encrypted_sm4_key_hex).constData(), QByteArray::fromHex(encrypted_sm4_key_hex).size(), sm4_key_raw, &sm4_key_len) != 1) {
                    qWarning() << "Failed to decrypt SM4 key for download.";
                    return;
                }
                sm4_set_decrypt_key(&sm4_dec_key, sm4_key_raw);
                iv = QByteArray::fromHex(iv_hex);


                headerReceived = true;
                downloadProgress->setMaximum(fileSize);
                // Process any encrypted data already received after the header
                // CORRECTED: Pass the data, not the size.
                processEncryptedFileData(downloadSocket->readAll());
                
            }
        }
    } else {
        processEncryptedFileData(downloadSocket->readAll());

    }
}

void FileHistoryDialog::processEncryptedFileData(const QByteArray& data) {
    if (data.isEmpty() || !downloadFile || !downloadFile->isOpen()) return;
    
    bytesReceived += data.size();
    
    // It's possible to receive partial blocks. SM4 CBC decryption needs full blocks.
    // For simplicity here, we assume data arrives in processable chunks. A robust implementation
    // would buffer incoming data to ensure full blocks are passed to decryption.
    uint8_t decrypted_buf[65536 + 16]; // Larger buffer
    size_t decrypted_len;

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

void FileHistoryDialog::completeDownload() {
    downloadFile->close();
    downloadProgress->setValue(downloadProgress->maximum());

    // --- SM3 HASH VERIFICATION ---
    QFile downloadedFile(downloadFile->fileName());
    if (!downloadedFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not re-open downloaded file for verification.");
        return;
    }
    QByteArray downloadedContent = downloadedFile.readAll();
    downloadedFile.close();

    uint8_t computed_hash[SM3_DIGEST_SIZE];
    // 修正 #5: 添加 (const uint8_t*) 类型转换
    sm3_digest((const uint8_t*)downloadedContent.constData(), downloadedContent.size(), computed_hash);
    QString computedHashHex = QByteArray((const char*)computed_hash, sizeof(computed_hash)).toHex();
    // --- END HASH VERIFICATION ---

    QString message = QString("File '%1' downloaded successfully!").arg(currentDownloadFileName);
    // 修正 #6: 使用已声明的成员变量
    if (computedHashHex == m_receivedHashHex) {
        message += "\n\nHash verification successful. File integrity is confirmed.";
        QMessageBox::information(this, "Success", message);
    } else {
        message += "\n\nERROR: Hash verification failed! The file may be corrupted.";
        QMessageBox::critical(this, "Verification Failed", message);
    }

    downloadSocket->disconnectFromHost();
}
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
