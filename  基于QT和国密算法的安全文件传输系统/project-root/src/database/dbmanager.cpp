#include "dbmanager.h"
#include <QUuid>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QFileInfo>
#include <QCryptographicHash>
#include <gmssl/sm3.h> 

// 【第一步：定义服务端“胡椒盐”】
// 这个密钥只存在于代码中，绝对不能泄露。
const static char* SERVER_PEPPER = "a_very_secret_and_long_string_for_admin_token_!@#$";


// SM3加盐哈希函数
QString DBManager::hashPasswordWithSalt(const QString &password, const QByteArray &salt) const {
    QByteArray data = salt + password.toUtf8();
    
    SM3_CTX ctx;
    uint8_t hash[SM3_DIGEST_SIZE];
    
    sm3_init(&ctx);
    sm3_update(&ctx, (const uint8_t*)data.constData(), data.size());
    sm3_finish(&ctx, hash);
    
    return QByteArray((const char*)hash, SM3_DIGEST_SIZE).toHex();
}

DBManager::DBManager(QObject *parent) 
    : QObject(parent), 
      m_db(QSqlDatabase::addDatabase("QSQLITE","my_unique_connection")) {
    if (!createConnection()) {
        qFatal("Failed to initialize database!");
    } 
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
    if (m_db.isOpen()) {
        m_db.close();
    }

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qCritical() << "SQLite driver not available!";
        return false;
    }

    QString dbPath = QCoreApplication::applicationDirPath() + "/users.db";
    bool dbExists = QFileInfo::exists(dbPath);
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qCritical() << "Cannot open database:" << m_db.lastError().text();
        return false;
    }
    
    QSqlQuery enableForeignKeys(m_db);
    if (!enableForeignKeys.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys";
    }

    QSqlQuery createTable(m_db);
    // ★ 核心修正 1：修改CREATE TABLE语句
    // 删除了 is_admin 列，添加了 admin_token 列
    bool success = createTable.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "salt TEXT NOT NULL,"
        "uuid TEXT UNIQUE NOT NULL,"
        "admin_token TEXT DEFAULT NULL)" // ★ 删除了 is_admin，添加了 admin_token
    );

    if (!success) {
        qCritical() << "Failed to create table:" << createTable.lastError().text();
        return false;
    }
    
    if (!dbExists) {
        qDebug() << "Database created for the first time. Adding default admin user.";
        QString adminUsername = "a";
        QString adminPassword = "a";
        QByteArray salt;
        salt.resize(16);
        for (int i = 0; i < 16; ++i) {
            salt[i] = QRandomGenerator::global()->generate() & 0xFF;
        }

        QString hashedPassword = hashPasswordWithSalt(adminPassword, salt);
        QString adminUuid = generateUuid();
        
        // ★ 核心修正 2：为默认管理员生成并插入admin_token
        QByteArray tokenData = adminUuid.toUtf8() + QByteArray(SERVER_PEPPER);
        SM3_CTX ctx;
        uint8_t hash[SM3_DIGEST_SIZE];
        sm3_init(&ctx);
        sm3_update(&ctx, (const uint8_t*)tokenData.constData(), tokenData.size());
        sm3_finish(&ctx, hash);
        QString adminToken = QByteArray((const char*)hash, SM3_DIGEST_SIZE).toHex();

        QSqlQuery insertAdmin(m_db);
        // ★ 核心修正 3：修改INSERT语句以匹配新表结构
        insertAdmin.prepare("INSERT INTO users (username, password, salt, uuid, admin_token) "
                            "VALUES (?, ?, ?, ?, ?)");
        insertAdmin.addBindValue(adminUsername);
        insertAdmin.addBindValue(hashedPassword);
        insertAdmin.addBindValue(salt.toHex());
        insertAdmin.addBindValue(adminUuid);
        insertAdmin.addBindValue(adminToken); // ★ 插入计算出的令牌

        if (!insertAdmin.exec()) {
            qCritical() << "Failed to add default admin user:" << insertAdmin.lastError().text();
            return false;
        } else {
            qDebug() << "Default admin user 'a' added successfully with UUID:" << adminUuid;
        }
    }

    QSqlQuery createFileTable(m_db);
    success = createFileTable.exec(
        "CREATE TABLE IF NOT EXISTS file_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_uuid TEXT NOT NULL,"
        "filename TEXT NOT NULL,"
        "file_size INTEGER NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(user_uuid) REFERENCES users(uuid))"
    );

    if (!success) {
        qCritical() << "Failed to create file_history table";
    }

    return true;
}
bool DBManager::recordFileTransfer(const QString &userUuid, 
                                 const QString &filename, 
                                 qint64 fileSize) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_db.isOpen() && !createConnection()) {
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO file_history (user_uuid, filename, file_size) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(userUuid);
    query.addBindValue(filename);
    query.addBindValue(fileSize);
    
    return query.exec();
}

QVector<QString> DBManager::getFileHistory(const QString &userUuid) {
    QMutexLocker locker(&m_mutex);
    QVector<QString> history;

    if (!m_db.isOpen() && !createConnection()) {
        return history;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT id, filename, file_size, timestamp FROM file_history "
                  "WHERE user_uuid = ? ORDER BY timestamp DESC");
    query.addBindValue(userUuid);

    if (query.exec()) {
        while (query.next()) {
            QString record = QString("%1 | %2 | %3 bytes | %4")
                .arg(query.value(0).toString())
                .arg(query.value(1).toString())
                .arg(query.value(2).toString())
                .arg(query.value(3).toString());
            history.append(record);
        }
    }

    return history;
}

// --- 新增方法的实现 ---
QVector<QString> DBManager::getAllFileHistory() {
    QMutexLocker locker(&m_mutex);
    QVector<QString> history;

    if (!m_db.isOpen() && !createConnection()) {
        return history;
    }

    QSqlQuery query(m_db);
    // 查询所有文件记录，按时间戳降序排列
    query.prepare("SELECT id, filename, file_size, timestamp FROM file_history ORDER BY timestamp DESC");

    if (query.exec()) {
        while (query.next()) {
            QString record = QString("%1 | %2 | %3 bytes | %4")
                .arg(query.value(0).toString())
                .arg(query.value(1).toString())
                .arg(query.value(2).toString())
                .arg(query.value(3).toString());
            history.append(record);
        }
    } else {
        qWarning() << "Failed to fetch all file history:" << query.lastError().text();
    }

    return history;
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
    QMutexLocker locker(&m_mutex);

    if (username.isEmpty() || password.isEmpty()) {
        return false;
    }

    QSqlQuery checkUser(m_db);
    checkUser.prepare("SELECT username FROM users WHERE username = ?");
    checkUser.addBindValue(username);
    
    if (!checkUser.exec() || checkUser.next()) {
        return false;
    }

    QByteArray salt;
    salt.resize(16);
    for (int i = 0; i < 16; ++i) {
        salt[i] = QRandomGenerator::global()->generate() & 0xFF;
    }

    // 【第二步：修改注册逻辑】
    uuid = generateUuid();
    QString hashedPassword = hashPasswordWithSalt(password, salt);
    
    QSqlQuery insertUser(m_db);
    // 注意：这里的SQL语句需要提前修改，将 is_admin 替换为 admin_token
    insertUser.prepare("INSERT INTO users (username, password, salt, uuid, admin_token) "
                      "VALUES (?, ?, ?, ?, ?)");
    insertUser.addBindValue(username);
    insertUser.addBindValue(hashedPassword);
    insertUser.addBindValue(salt.toHex());
    insertUser.addBindValue(uuid);

    if (isAdmin) {
        // 如果是管理员，则计算并绑定权限令牌
        QByteArray tokenData = uuid.toUtf8() + QByteArray(SERVER_PEPPER);
        
        SM3_CTX ctx;
        uint8_t hash[SM3_DIGEST_SIZE];
        sm3_init(&ctx);
        sm3_update(&ctx, (const uint8_t*)tokenData.constData(), tokenData.size());
        sm3_finish(&ctx, hash);

        QString adminToken = QByteArray((const char*)hash, SM3_DIGEST_SIZE).toHex();
        insertUser.addBindValue(adminToken);
    } else {
        // 如果是普通用户，则绑定一个空值
        insertUser.addBindValue(QVariant(QVariant::String)); // 插入NULL
    }
    
    return insertUser.exec();
}

bool DBManager::loginUser(const QString &username, 
                        const QString &password,
                        QString &uuid) {
    QSqlQuery query(m_db);
    query.prepare("SELECT password, salt, uuid FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    QString storedHash = query.value(0).toString();
    QByteArray salt = QByteArray::fromHex(query.value(1).toByteArray());
    uuid = query.value(2).toString();
    
    QString inputHash = hashPasswordWithSalt(password, salt);
    
    return (storedHash == inputHash);
}


// 【第三步：修改管理员验证逻辑】
bool DBManager::isAdminUser(const QString &uuid) {
    QMutexLocker locker(&m_mutex);
    if (!m_db.isOpen() && !createConnection()) {
        qWarning() << "Failed to open database in isAdminUser";
        return false;
    }

    QSqlQuery query(m_db);
    // 注意：查询的字段从 is_admin 变为 admin_token
    query.prepare("SELECT admin_token FROM users WHERE uuid = ?");
    query.addBindValue(uuid);

    if (!query.exec() || !query.next()) {
        return false; // 用户不存在
    }

    QString storedToken = query.value(0).toString();
    if (storedToken.isEmpty()) {
        return false; // 令牌为空，是普通用户
    }

    // 在服务器端重新计算期望的令牌
    QByteArray tokenData = uuid.toUtf8() + QByteArray(SERVER_PEPPER);
    SM3_CTX ctx;
    uint8_t hash[SM3_DIGEST_SIZE];
    sm3_init(&ctx);
    sm3_update(&ctx, (const uint8_t*)tokenData.constData(), tokenData.size());
    sm3_finish(&ctx, hash);
    QString expectedToken = QByteArray((const char*)hash, SM3_DIGEST_SIZE).toHex();

    // 只有存储的令牌与实时计算出的期望令牌完全一致，才是管理员
    return (storedToken == expectedToken);
}

bool DBManager::isFileOwner(const QString &filename, const QString &userUuid) {
    QMutexLocker locker(&m_mutex);
    if (!m_db.isOpen() && !createConnection()) {
        qWarning() << "Failed to open database in isFileOwner";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT user_uuid FROM file_history WHERE filename = ? ORDER BY timestamp DESC LIMIT 1");
    query.addBindValue(filename);

    if (query.exec() && query.next()) {
        return query.value(0).toString() == userUuid;
    }
    return false;
}
