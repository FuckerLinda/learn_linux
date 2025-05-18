#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

// 登录页面
class LoginPage : public QWidget {
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr);
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    
signals:
    void loginRequested(const QString &user, const QString &pass);
    void showRegister();
};

// 注册页面
class RegisterPage : public QWidget {
    Q_OBJECT
public:
    explicit RegisterPage(QWidget *parent = nullptr);
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLineEdit *confirmPasswordInput;
    QLineEdit *inviteCodeInput;  // 新增
    
signals:
    void registerRequested(const QString &user, const QString &pass, const QString &confirm, const QString &inviteCode);
    void showLogin();
};
