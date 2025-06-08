#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include "dbmanager.h"
#include <QProgressBar>
#include <QDialog>
#include <QListWidget>
#include <gmssl/sm2.h>
#include <gmssl/sm3.h>
#include <gmssl/sm4.h>
#include <gmssl/rand.h>

class FileHistoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit FileHistoryDialog(const QString &userUuid, QWidget *parent = nullptr);
    virtual ~FileHistoryDialog();
    
private slots:
    void onDownloadClicked();
    void onDownloadConnected();
    void onDownloadReadyRead();
    void onDownloadDisconnected();
    void onDownloadError(QAbstractSocket::SocketError error);
    
private:
    void completeDownload();
    void processEncryptedFileData(const QByteArray& data);
    
private:
    QListWidget *fileList;
    QString currentUserUuid;
    QTcpSocket *downloadSocket;
    QFile *downloadFile;
    qint64 fileSize;
    qint64 bytesReceived;
    QString currentDownloadFileName;
    QProgressBar *downloadProgress;
    bool headerReceived;
    // Crypto state
    SM4_KEY sm4_dec_key;
    QByteArray iv;
    QString m_receivedHashHex;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void onSendButtonClicked();
    void onConnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void handleLogin(const QString &user, const QString &pass);
    void handleRegister(const QString &user, const QString &pass, 
                       const QString &confirm, const QString &inviteCode);
    void showFileTransferPage(const QString &uuid);
    void onBytesWritten(qint64 bytes);
    void onViewHistoryClicked();
    void onLogoutButtonClicked(); // <-- 新增的槽函数

private:
    QWidget* createFileTransferPage();
    void updateUIForPermissions(bool isAdmin); 
    
    QTcpSocket *socket;
    QString selectedFilePath;
    QPushButton *selectButton;
    QPushButton *sendButton;
    QPushButton *logoutButton; // <-- 新增的按钮指针
    QLabel *fileLabel;
    QLabel *userInfoLabel;
    QStackedWidget *stackedWidget;
    DBManager *dbManager;
    QFile fileToSend;
    qint64 fileSize;
    qint64 bytesWritten;
    QProgressBar *progressBar;
    QString currentUserUuid;
    QPushButton *viewHistoryBtn;
    bool m_headerHandlingDone;
    // Crypto state for uploads
    SM2_KEY sm2_key;
    uint8_t sm4_key[16];
    uint8_t iv[16];
    QByteArray m_uploadFileHash; // <-- 添加用于暂存哈希的成员
};


#endif // MAINWINDOW_H
