#include "mainwindow.h"
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), socket(new QTcpSocket(this)) {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    QPushButton *selectButton = new QPushButton("Select File", centralWidget);
    sendButton = new QPushButton("Send File", centralWidget);
    sendButton->setEnabled(false);
    fileLabel = new QLabel("No file selected", centralWidget);
    
    layout->addWidget(selectButton);
    layout->addWidget(fileLabel);
    layout->addWidget(sendButton);
    
    setCentralWidget(centralWidget);
    
    connect(selectButton, &QPushButton::clicked, this, &MainWindow::onSelectButtonClicked);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &MainWindow::onErrorOccurred);
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
