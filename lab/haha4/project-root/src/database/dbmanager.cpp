#include "dbmanager.h"
#include <QUuid>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QRandomGenerator>

DBManager::DBManager(QObject *parent) 
    : QObject(parent), 
      m_db(QSqlDatabase::addDatabase("QSQLITE","my_unique_connection")) {
    if (!createConnection()) {
        qFatal("Failed to initialize database!"); // 致命错误，终止程序
    } 
    QDir().mkpath(QCoreApplication::applicationDirPath());
    //createConnection();  // 调用连接方法(已重复，故删去)
}

DBManager::~DBManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DBManager::isConnected() const {
    return m_db.isOpen();
}

QString DBManager::generateUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool DBManager::createConnection() {
    qDebug() << "Initializing database connection...";
    
    // 先关闭已有连接
    if (m_db.isOpen()) {
        m_db.close();
    }

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qCritical() << "SQLite driver not available!";
        return false;
    }

    // 使用唯一连接名（添加随机后缀避免冲突）
    QString connectionName = "my_unique_connection_" + QString::number(QRandomGenerator::global()->generate());
    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
									 //
    QString dbPath = QCoreApplication::applicationDirPath() + "/users.db";
    qDebug() << "Database path:" << dbPath;
    
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qCritical() << "Cannot open database:" << m_db.lastError().text();
        qCritical() << "Database path:" << dbPath;  // 添加路径输出
        return false;
    }
    qDebug() << "Database opened successfully. Connection status:" << m_db.isOpen();
    
    // 添加权限检查
    QFileInfo fi(dbPath);
    if (!fi.isWritable()) {
        qCritical() << "Database file is not writable";
        return false;
    }

    QSqlQuery enableForeignKeys(m_db);
    if (!enableForeignKeys.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:" 
                  << enableForeignKeys.lastError().text();
    }

    QSqlQuery createTable(m_db);
    bool success = createTable.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "uuid TEXT UNIQUE NOT NULL,"
        "is_admin INTEGER DEFAULT 0)"  // 新增管理员标识列

    );

    if (!success) {
        qCritical() << "Failed to create table:" << createTable.lastError().text();
        m_db.close();
        return false;
    }

    return true;
}

QString DBManager::hashPassword(const QString &password) const {
    return QCryptographicHash::hash(
        password.toUtf8(), 
        QCryptographicHash::Sha256
    ).toHex();
}

bool DBManager::registerUser(const QString &username, 
                           const QString &password,
                           QString &uuid,
			   bool isAdmin) {

    QMutexLocker locker(&m_mutex); // 添加线程安全

    // 添加详细的连接状态检查
    if (!m_db.isOpen()) {
        qCritical() << "Database is not open!";
        return false;
    }
    // 参数合法性检查
    if (username.isEmpty() || password.isEmpty()) {
        qCritical() << "Invalid username or password";
        return false;
    }
    // 添加有效性检查
    if (!m_db.isValid() && !createConnection()) {
        qCritical() << "Invalid database connection";
        return false;
    }
    QSqlQuery checkUser(m_db);
    checkUser.prepare("SELECT username FROM users WHERE username = ?");
    checkUser.addBindValue(username);
    
    if (!checkUser.exec()) {
        qCritical() << "Query error:" << checkUser.lastError().text();
        return false;
    }
    
    if (checkUser.next()) {
        qWarning() << "Username already exists";
        return false;
    }

    uuid = generateUuid();
    QString hashedPassword = hashPassword(password);
    
    QSqlQuery insertUser(m_db);
    insertUser.prepare("INSERT INTO users (username, password, uuid, is_admin) "
                      "VALUES (?, ?, ?, ?)");
    insertUser.addBindValue(username);
    insertUser.addBindValue(hashedPassword);
    insertUser.addBindValue(uuid);
    insertUser.addBindValue(isAdmin ? 1 : 0);  // 绑定管理员状态
    
    if (!insertUser.exec()) {
        qCritical() << "Insert failed:" << insertUser.lastError().text();
        return false;
    }
    
    return true;
}

bool DBManager::loginUser(const QString &username, 
                        const QString &password,
                        QString &uuid) {
    if (!m_db.isOpen() && !createConnection()) {
        return false;
    }

    QSqlQuery query(m_db);
    if (!query.prepare("SELECT password, uuid FROM users WHERE username = ?")) {
        qCritical() << "Prepare failed:" << query.lastError().text();
        return false;
    }
    
    query.addBindValue(username);
    
    if (!query.exec()) {
        qCritical() << "Query failed:" << query.lastError().text();
        return false;
    }
    
    if (!query.next()) {
        qWarning() << "User not found:" << username;
        return false;
    }
    
    QString storedHash = query.value(0).toString();
    QString inputHash = hashPassword(password);
    
    if (storedHash != inputHash) {
        qWarning() << "Password mismatch for user:" << username;
        return false;
    }
    
    uuid = query.value(1).toString();
    return true;
}
