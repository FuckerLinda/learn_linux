#include "authwidgets.h"

LoginPage::LoginPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username");
    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    
    QPushButton *loginBtn = new QPushButton("Login", this);
    QPushButton *toRegisterBtn = new QPushButton("Register", this);
    
    layout->addWidget(new QLabel("Login", this));
    layout->addWidget(usernameInput);
    layout->addWidget(passwordInput);
    layout->addWidget(loginBtn);
    layout->addWidget(toRegisterBtn);
    
    connect(loginBtn, &QPushButton::clicked, [this] {
        emit loginRequested(usernameInput->text(), passwordInput->text());
    });
    
    connect(toRegisterBtn, &QPushButton::clicked, this, &LoginPage::showRegister);
}

RegisterPage::RegisterPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    usernameInput = new QLineEdit(this);
    usernameInput->setPlaceholderText("Username");
    passwordInput = new QLineEdit(this);
    passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);
    confirmPasswordInput = new QLineEdit(this);
    confirmPasswordInput->setPlaceholderText("Confirm Password");
    confirmPasswordInput->setEchoMode(QLineEdit::Password);
    
    QPushButton *registerBtn = new QPushButton("Register", this);
    QPushButton *toLoginBtn = new QPushButton("Back to Login", this);
   
    inviteCodeInput = new QLineEdit(this);  // 新增
    inviteCodeInput->setPlaceholderText("Invite Code (optional)");
    
    // 在布局中添加
    layout->addWidget(inviteCodeInput);
    layout->addWidget(new QLabel("Register", this));
    layout->addWidget(usernameInput);
    layout->addWidget(passwordInput);
    layout->addWidget(confirmPasswordInput);
    layout->addWidget(registerBtn);
    layout->addWidget(toLoginBtn);
    
    connect(registerBtn, &QPushButton::clicked, [this] {
        emit registerRequested(usernameInput->text(), 
                             passwordInput->text(),
                             confirmPasswordInput->text(),
	                     inviteCodeInput->text());  // 新增参数
    });
    
    connect(toLoginBtn, &QPushButton::clicked, this, &RegisterPage::showLogin);
}
