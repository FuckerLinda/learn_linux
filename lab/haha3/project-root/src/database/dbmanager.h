#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

class DBManager : public QObject {
    Q_OBJECT
public:

    static DBManager& instance() {
        static DBManager instance;
        return instance;
    }

    // 删除拷贝构造函数和赋值运算符
    DBManager(const DBManager&) = delete;
    void operator=(const DBManager&) = delete;

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

private:
    explicit DBManager(QObject *parent = nullptr); // 改为私有
    ~DBManager();
    QString hashPassword(const QString &password) const;
    QSqlDatabase m_db;
    QMutex m_mutex;  // 添加互斥锁
};

#endif // DBMANAGER_H
