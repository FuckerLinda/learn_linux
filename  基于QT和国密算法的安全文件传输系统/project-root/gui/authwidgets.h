#ifndef AUTHWIDGETS_H
#define AUTHWIDGETS_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

// 登录页面类
class LoginPage : public QWidget {
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr); // 构造函数
    QLineEdit *usernameInput;  // 用户名输入框
    QLineEdit *passwordInput;  // 密码输入框
    
signals:
    void loginRequested(const QString &user, const QString &pass); // 登录请求信号
    void showRegister(); // 显示注册页面信号
};

// 注册页面类
class RegisterPage : public QWidget {
    Q_OBJECT
public:
    explicit RegisterPage(QWidget *parent = nullptr); // 构造函数
    QLineEdit *usernameInput;       // 用户名输入框
    QLineEdit *passwordInput;       // 密码输入框
    QLineEdit *confirmPasswordInput; // 确认密码输入框
    QLineEdit *inviteCodeInput;     // 邀请码输入框
    
signals:
    void registerRequested(const QString &user, const QString &pass, 
                          const QString &confirm, const QString &inviteCode); // 注册请求信号
    void showLogin(); // 显示登录页面信号
};

#endif // AUTHWIDGETS_H
