#include "authwidgets.h"

// 登录页面构造函数
LoginPage::LoginPage(QWidget *parent) : QWidget(parent) {
    // 创建垂直布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // 创建输入框和按钮
    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username"); // 设置占位文本
    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password); // 设置为密码模式
    
    QPushButton *loginBtn = new QPushButton("Login", this);
    QPushButton *toRegisterBtn = new QPushButton("Register", this);
    
    // 添加控件到布局
    layout->addWidget(new QLabel("Login", this));
    layout->addWidget(usernameInput);
    layout->addWidget(passwordInput);
    layout->addWidget(loginBtn);
    layout->addWidget(toRegisterBtn);
    
    // 连接登录按钮信号
    connect(loginBtn, &QPushButton::clicked, [this] {
        // 发出登录请求信号，传递用户名和密码
        emit loginRequested(usernameInput->text(), passwordInput->text());
    });
    
    // 连接注册按钮信号
    connect(toRegisterBtn, &QPushButton::clicked, this, &LoginPage::showRegister);
}

// 注册页面构造函数
RegisterPage::RegisterPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // 创建输入框
    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username");
    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    confirmPasswordInput = new QLineEdit(this);
    confirmPasswordInput->setPlaceholderText("Confirm Password");
    confirmPasswordInput->setEchoMode(QLineEdit::Password);
    inviteCodeInput = new QLineEdit(this);
    inviteCodeInput->setPlaceholderText("Invite Code (optional)"); // 邀请码输入框
    
    // 创建按钮
    QPushButton *registerBtn = new QPushButton("Register", this);
    QPushButton *toLoginBtn = new QPushButton("Back to Login", this);
   
    // 添加控件到布局
    layout->addWidget(new QLabel("Register", this));
    layout->addWidget(usernameInput);
    layout->addWidget(passwordInput);
    layout->addWidget(confirmPasswordInput);
    layout->addWidget(inviteCodeInput);  // 添加邀请码输入框
    layout->addWidget(registerBtn);
    layout->addWidget(toLoginBtn);
    
    // 连接注册按钮信号
    connect(registerBtn, &QPushButton::clicked, [this] {
        // 发出注册请求信号，传递所有输入信息
        emit registerRequested(usernameInput->text(), 
                             passwordInput->text(),
                             confirmPasswordInput->text(),
                             inviteCodeInput->text());
    });
    
    // 连接返回登录按钮信号
    connect(toLoginBtn, &QPushButton::clicked, this, &RegisterPage::showLogin);
}
