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
#include <QProgressBar> // 添加进度条头文件


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
    void onBytesWritten(qint64 bytes);  // 添加此声明

private:
    QTcpSocket *socket;
    QString selectedFilePath;
    QPushButton *sendButton;
    QLabel *fileLabel;
    QLabel *userInfoLabel;
    //add week3
    QStackedWidget *stackedWidget;
    DBManager *dbManager;
    QWidget* createFileTransferPage(); // 声明方法
    QFile fileToSend;                  // 添加文件对象
    qint64 fileSize;                   // 文件大小
    qint64 bytesWritten;               // 已写入字节数
    QProgressBar *progressBar; // 添加进度条

};

#endif // MAINWINDOW_H
