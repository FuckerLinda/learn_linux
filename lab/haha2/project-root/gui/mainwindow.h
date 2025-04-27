#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>
#include <QPushButton>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private slots:
    void onSelectButtonClicked();
    void onSendButtonClicked();
    void onConnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
private:
    QTcpSocket *socket;
    QString selectedFilePath;
    QPushButton *sendButton;
    QLabel *fileLabel;
};

#endif // MAINWINDOW_H
