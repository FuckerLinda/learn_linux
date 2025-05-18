#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include "authwidgets.h"
#include <QThread>  // 添加此行

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),dbManager(&DBManager::instance()), socket(new QTcpSocket(this)) {

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
}

QWidget* MainWindow::createFileTransferPage() {
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);

    // 从原构造函数迁移的组件创建代码
    QPushButton *selectButton = new QPushButton("Select File", page);
    sendButton = new QPushButton("Send File", page);
    sendButton->setEnabled(false);
    fileLabel = new QLabel("No file selected", page);
    QLabel *userInfoLabel = new QLabel("User: Not logged in", page);

    layout->addWidget(userInfoLabel);
    layout->addWidget(selectButton);
    layout->addWidget(fileLabel);
    layout->addWidget(sendButton);

    // 迁移信号连接（需要将lambda中的this改为捕获page）
    connect(selectButton, &QPushButton::clicked, [this, page]() {
        selectedFilePath = QFileDialog::getOpenFileName(page, "Select File");
        if (!selectedFilePath.isEmpty()) {
            fileLabel->setText(selectedFilePath);
            sendButton->setEnabled(true);
        }
    });

    return page;
}

void MainWindow::handleLogin(const QString &user, const QString &pass) {
    QString uuid;
    if (dbManager->loginUser(user, pass, uuid)) {
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
    socket->connectToHost("localhost", 1234);
}

void MainWindow::onConnected() {
    QFile file(selectedFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open file!");
        return;
    }
    QByteArray data = file.readAll();
    socket->write(data);
    file.close();
    socket->disconnectFromHost();
    QMessageBox::information(this, "Success", "File sent successfully.");
}

void MainWindow::onErrorOccurred(QAbstractSocket::SocketError error) {
    QMessageBox::critical(this, "Error", "Connection error: " + socket->errorString());
}
