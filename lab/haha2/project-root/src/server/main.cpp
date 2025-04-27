#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDateTime>
#include <QDebug>

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
        QString filename = QDateTime::currentDateTime().toString("'received_'yyyyMMdd_hhmmsszzz'.dat'");
        QFile *file = new QFile(filename);
        
        if (!file->open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open file for writing:" << filename;
            delete file;
            socket->close();
            return;
        }

        QObject::connect(socket, &QTcpSocket::readyRead, [socket, file]() {
            file->write(socket->readAll());
        });

        QObject::connect(socket, &QTcpSocket::disconnected, [socket, file, filename]() {
            file->close();
            delete file;
            socket->deleteLater();
            qInfo() << "File saved as:" << filename;
        });
    });

    return app.exec();
}
