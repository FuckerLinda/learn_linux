#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QThread>  // 添加此行
#include <QObject>  // 添加此行

// 新增线程处理类
class SocketHandler : public QObject {
    Q_OBJECT
public:
    explicit SocketHandler(QTcpSocket *socket, QObject *parent = nullptr)
        : QObject(parent), socket(socket) {
        // 生成唯一文件名
        filename = QDateTime::currentDateTime().toString("'received_'yyyyMMdd_hhmmsszzz'.dat'");
        file.setFileName(filename);
    }

public slots:
    void handleConnection() {
        // 打开文件
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open file for writing:" << filename;
            socket->close();
            deleteLater();
            return;
        }

        // 绑定数据读取事件
        connect(socket, &QTcpSocket::readyRead, this, &SocketHandler::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &SocketHandler::onDisconnected);
    }

private slots:
    void onReadyRead() {
        file.write(socket->readAll());
    }

    void onDisconnected() {
        file.close();
        socket->deleteLater();
        qInfo() << "File saved as:" << filename;
        deleteLater(); // 自毁对象
    }

private:
    QTcpSocket *socket;
    QFile file;
    QString filename;
};


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTcpServer server;
    if (!server.listen(QHostAddress::Any, 1234)) {
        qCritical() << "Server could not start!";
        return 1;
    }
    qInfo() << "Server listening on port 1234...";

    QObject::connect(&server, &QTcpServer::newConnection, [&]() {
        QTcpSocket *socket = server.nextPendingConnection();

        // 创建子线程
        QThread *thread = new QThread;
        SocketHandler *handler = new SocketHandler(socket);
        handler->moveToThread(thread);

        // 线程结束时自动清理资源
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        QObject::connect(thread, &QThread::finished, handler, &SocketHandler::deleteLater);

        // 启动线程处理逻辑
        QObject::connect(thread, &QThread::started, handler, &SocketHandler::handleConnection);
        thread->start();
    });

    return app.exec();
}

#include "main.moc" // 确保包含元对象（如果使用分离的类定义）
