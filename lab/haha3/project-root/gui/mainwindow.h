#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>
#include <QPushButton>
#include <QLabel>
//add week3
#include <QStackedWidget>
#include "dbmanager.h" 

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    //modify week3
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void onSelectButtonClicked();
    void onSendButtonClicked();
    void onConnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    //add week3
    void handleLogin(const QString &user, const QString &pass);
    void handleRegister(const QString &user, const QString &pass, const QString &confirm, const QString &inviteCode);
    void showFileTransferPage(const QString &uuid);
private:
    QTcpSocket *socket;
    QString selectedFilePath;
    QPushButton *sendButton;
    QLabel *fileLabel;
    //add week3
    QStackedWidget *stackedWidget;
    DBManager *dbManager;
    QWidget* createFileTransferPage(); // 声明方法
};

#endif // MAINWINDOW_H
