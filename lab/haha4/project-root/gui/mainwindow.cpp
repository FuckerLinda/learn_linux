#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include "authwidgets.h"
#include <QThread>  // 添加此行
#include <QProgressBar> // 添加进度条头文件

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),dbManager(&DBManager::instance()), socket(new QTcpSocket(this)),fileSize(0),bytesWritten(0) {

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // 页面1：登录/注册选择
    QWidget *authChoicePage = new QWidget;
    QVBoxLayout *authLayout = new QVBoxLayout(authChoicePage);
    QPushButton *loginBtn = new QPushButton("Login", this);
    QPushButton *registerBtn = new QPushButton("Register", this);
    authLayout->addWidget(loginBtn);
    authLayout->addWidget(registerBtn);
    
    // 页面2：登录表单
    LoginPage *loginPage = new LoginPage;
    // 页面3：注册表单
    RegisterPage *registerPage = new RegisterPage;
    // 页面4：原有文件传输界面（需稍作调整）
    QWidget *fileTransferPage = createFileTransferPage(); // 将原有界面封装为函数
    
    stackedWidget->addWidget(authChoicePage);  // Index 0
    stackedWidget->addWidget(loginPage);       // Index 1
    stackedWidget->addWidget(registerPage);    // Index 2
    stackedWidget->addWidget(fileTransferPage);// Index 3

    // 信号连接
    connect(loginBtn, &QPushButton::clicked, [this] {
        stackedWidget->setCurrentIndex(1);
    });
    
    connect(registerBtn, &QPushButton::clicked, [this] {
        stackedWidget->setCurrentIndex(2);
    });
    
    connect(loginPage, &LoginPage::loginRequested, 
           this, &MainWindow::handleLogin);
    
    
    connect(loginPage, &LoginPage::showRegister, [this] {
        stackedWidget->setCurrentIndex(2);
    });
    

    connect(registerPage, &RegisterPage::registerRequested,
           this, &MainWindow::handleRegister);
    

    connect(registerPage, &RegisterPage::showLogin, [this] {
        stackedWidget->setCurrentIndex(1);
    });


    // 添加 socket 信号连接
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, &QTcpSocket::bytesWritten, this, &MainWindow::onBytesWritten);
    connect(socket, &QTcpSocket::errorOccurred, this, &MainWindow::onErrorOccurred);

}

QWidget* MainWindow::createFileTransferPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    // 从原构造函数迁移的组件创建代码
    QPushButton *selectButton = new QPushButton("Select File", page);
    sendButton = new QPushButton("Send File", page);
    sendButton->setEnabled(false);
    //fileLabel = new QLabel("No file selected", page);
    //QLabel *userInfoLabel = new QLabel("User: Not logged in", page);
    fileLabel = new QLabel("No file selected", page); // 假设 fileLabel 已经是成员变量
    userInfoLabel = new QLabel("User: Not logged in", page); // <<< 修改此行：赋值给成员变量
   

    // 添加进度条
    progressBar = new QProgressBar(page); // 正确声明进度条
    progressBar->setVisible(false);
    
    //layout->addWidget(userInfoLabel);
    layout->addWidget(this->userInfoLabel); // 使用成员变量
    layout->addWidget(selectButton);
    layout->addWidget(fileLabel);
    layout->addWidget(progressBar); // 添加进度条到布局
    layout->addWidget(sendButton);

    // 迁移信号连接（需要将lambda中的this改为捕获page）
    connect(selectButton, &QPushButton::clicked, [this, page]() {
        selectedFilePath = QFileDialog::getOpenFileName(page, "Select File");
        if (!selectedFilePath.isEmpty()) {
            fileLabel->setText(selectedFilePath);
            sendButton->setEnabled(true);
        }
    });
   
    // 添加 sendButton 的信号连接
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);

    // 在 onBytesWritten 中更新进度
    progressBar->setValue(static_cast<int>((bytesWritten * 100) / fileSize)); 

    return page;
}

void MainWindow::handleLogin(const QString &user, const QString &pass) {
    QString uuid;
    if (dbManager->loginUser(user, pass, uuid)) {
        if (userInfoLabel) { // 假设 userInfoLabel 现在是成员变量
            userInfoLabel->setText("用户 UUID: " + uuid); // 或者 "用户: " + user
        }
	showFileTransferPage(uuid);
        QMessageBox::information(this, "Success",
            "Logged in! Your UUID: " + uuid);
    } else {
        QMessageBox::warning(this, "Error", "Invalid credentials");
    }
}

void MainWindow::handleRegister(const QString &user,
                               const QString &pass,
                               const QString &confirm,
			       const QString &inviteCode) {
    qDebug() << "Enter handleRegister";
    qDebug() << "User:" << user << "Pass:" << pass << "Confirm:" << confirm;
    qDebug() << "Current thread:" << QThread::currentThread();
    bool isAdmin = (inviteCode == "114514");
    if (pass != confirm) {
        QMessageBox::warning(this, "Error", "Passwords do not match");
        return;
    }

    QString uuid;
    if (dbManager->registerUser(user, pass, uuid,isAdmin)) {
    qDebug() << "Registration succeeded. UUID:" << uuid;
    QMessageBox::information(this, "Success",
            "Registered! Your UUID: " + uuid);
        stackedWidget->setCurrentIndex(1); // 跳转回登录页
    } else {
        QMessageBox::warning(this, "Error", "Registration failed");
	qDebug() << "Registration failed";
    }
}

void MainWindow::showFileTransferPage(const QString &uuid) {
    stackedWidget->setCurrentIndex(3);
    // 可以在这里保存uuid到成员变量供后续使用
    if (userInfoLabel) { // 确保标签存在
        userInfoLabel->setText("用户 UUID: " + uuid); // <<< 添加此行以更新标签
    }
}


void MainWindow::onSelectButtonClicked() {
    selectedFilePath = QFileDialog::getOpenFileName(this, "Select File");
    if (!selectedFilePath.isEmpty()) {
        fileLabel->setText(selectedFilePath);
        sendButton->setEnabled(true);
    }
}

void MainWindow::onSendButtonClicked() {
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "No file selected.");
        return;
    }

    // 重置文件传输状态
    fileToSend.setFileName(selectedFilePath);
    bytesWritten = 0;

    if (!fileToSend.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file: " + fileToSend.errorString());
        return;
    }

    fileSize = fileToSend.size();
    socket->connectToHost("localhost", 1234);

}

void MainWindow::onConnected() {

    // 发送文件头信息 (文件名和大小)
    QFileInfo fileInfo(selectedFilePath);
    QString header = QString("FILE:%1:%2\n").arg(fileInfo.fileName()).arg(fileSize);
    socket->write(header.toUtf8());
   
    // 发送第一块数据
    QByteArray chunk = fileToSend.read(65536);
    socket->write(chunk);
    bytesWritten = chunk.size(); // 更新已写入字节数
}

void MainWindow::onBytesWritten(qint64 bytes) {
    // 更新已写入字节数（不包括文件头）
    static bool headerSent = false;
    if (!headerSent) {
        // 文件头已发送完成
        headerSent = true;
        return;
    }
    
    // 继续发送文件内容
    if (bytesWritten < fileSize) {
        QByteArray chunk = fileToSend.read(65536); // 64KB 分块
        qint64 bytesToWrite = qMin(chunk.size(), static_cast<int>(fileSize - bytesWritten));
        
        if (bytesToWrite > 0) {
            socket->write(chunk.constData(), bytesToWrite);
            bytesWritten += bytesToWrite;
        }
    } else {
        // 文件发送完成
        fileToSend.close();
        socket->disconnectFromHost();
        QMessageBox::information(this, "Success", "File sent successfully!");
        headerSent = false; // 重置状态
    }
}


void MainWindow::onErrorOccurred(QAbstractSocket::SocketError error) {
    QMessageBox::critical(this, "Error", "Connection error: " + socket->errorString());
}
