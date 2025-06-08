#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <gmssl/sm3.h> // 添加SM3支持

class DBManager : public QObject {
    Q_OBJECT
public:
    static DBManager& instance() {
        static DBManager instance;
        return instance;
    }
    
    DBManager(const DBManager&) = delete;
    void operator=(const DBManager&) = delete;
    bool isAdminUser(const QString &uuid);
    bool isFileOwner(const QString &filename, const QString &userUuid);
    bool createConnection();
    bool isConnected() const;
    
    bool registerUser(const QString &username, 
                    const QString &password, 
                    QString &uuid,
                    bool isAdmin = false);
                    
    bool loginUser(const QString &username,
                 const QString &password,
                 QString &uuid);

    static QString generateUuid();
    bool recordFileTransfer(const QString &userUuid, 
                          const QString &filename, 
                          qint64 fileSize);
    QVector<QString> getFileHistory(const QString &userUuid);
    QVector<QString> getAllFileHistory(); // <-- 新增的方法

private:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();
    QString hashPassword(const QString &password) const;
    QSqlDatabase m_db;
    mutable QMutex m_mutex;
    QString hashPasswordWithSalt(const QString &password, const QByteArray &salt) const;
};

#endif // DBMANAGER_H
