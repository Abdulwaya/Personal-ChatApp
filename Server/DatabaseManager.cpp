#include "DatabaseManager.h"
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

DatabaseManager::DatabaseManager() {
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect() {
    QMutexLocker locker(&m_mutex);
    
    m_db.setHostName("localhost");
    m_db.setDatabaseName("chatapp");
    m_db.setUserName("root");
    m_db.setPassword("Arainyday_06");
    
    if (!m_db.open()) {
        qDebug() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }
    
    createTables();
    return true;
}

void DatabaseManager::disconnect() {
    QMutexLocker locker(&m_mutex);
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void DatabaseManager::createTables() {
    QSqlQuery query(m_db);
    
    // Users table
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INT AUTO_INCREMENT PRIMARY KEY,"
               "username VARCHAR(50) UNIQUE NOT NULL,"
               "password VARCHAR(255) NOT NULL,"
               "is_online BOOLEAN DEFAULT FALSE,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
    
    // Groups table
    query.exec("CREATE TABLE IF NOT EXISTS groups ("
               "id INT AUTO_INCREMENT PRIMARY KEY,"
               "group_name VARCHAR(50) UNIQUE NOT NULL,"
               "admin_username VARCHAR(50) NOT NULL,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY (admin_username) REFERENCES users(username))");
    
    // Group members table
    query.exec("CREATE TABLE IF NOT EXISTS group_members ("
               "group_id INT,"
               "username VARCHAR(50),"
               "joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "PRIMARY KEY (group_id, username),"
               "FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE,"
               "FOREIGN KEY (username) REFERENCES users(username) ON DELETE CASCADE)");
    
    // Private messages table
    query.exec("CREATE TABLE IF NOT EXISTS private_messages ("
               "id INT AUTO_INCREMENT PRIMARY KEY,"
               "sender VARCHAR(50) NOT NULL,"
               "recipient VARCHAR(50) NOT NULL,"
               "content TEXT NOT NULL,"
               "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY (sender) REFERENCES users(username) ON DELETE CASCADE,"
               "FOREIGN KEY (recipient) REFERENCES users(username) ON DELETE CASCADE)");
    
    // Group messages table
    query.exec("CREATE TABLE IF NOT EXISTS group_messages ("
               "id INT AUTO_INCREMENT PRIMARY KEY,"
               "sender VARCHAR(50) NOT NULL,"
               "group_id INT NOT NULL,"
               "content TEXT NOT NULL,"
               "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY (sender) REFERENCES users(username) ON DELETE CASCADE,"
               "FOREIGN KEY (group_id) REFERENCES groups(id) ON DELETE CASCADE)");
}

bool DatabaseManager::registerUser(const QString& username, const QString& password) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    
    return query.exec();
}

bool DatabaseManager::loginUser(const QString& username, const QString& password) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("SELECT * FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    
    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}

QStringList DatabaseManager::getAllUsers() {
    QMutexLocker locker(&m_mutex);
    QStringList users;
    QSqlQuery query("SELECT username FROM users", m_db);
    
    while (query.next()) {
        users.append(query.value(0).toString());
    }
    return users;
}

void DatabaseManager::setUserOnlineStatus(const QString& username, bool online) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("UPDATE users SET is_online = :online WHERE username = :username");
    query.bindValue(":online", online);
    query.bindValue(":username", username);
    query.exec();
}

bool DatabaseManager::createGroup(const QString& groupName, const QString& adminUsername) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("INSERT INTO groups (group_name, admin_username) VALUES (:name, :admin)");
    query.bindValue(":name", groupName);
    query.bindValue(":admin", adminUsername);
    
    if (!query.exec()) return false;
    
    int groupId = query.lastInsertId().toInt();
    
    query.prepare("INSERT INTO group_members (group_id, username) VALUES (:gid, :user)");
    query.bindValue(":gid", groupId);
    query.bindValue(":user", adminUsername);
    
    return query.exec();
}

QStringList DatabaseManager::getUserGroups(const QString& username) {
    QMutexLocker locker(&m_mutex);
    QStringList groups;
    QSqlQuery query(m_db);
    
    query.prepare("SELECT g.group_name FROM groups g "
                  "JOIN group_members gm ON g.id = gm.group_id "
                  "WHERE gm.username = :username");
    query.bindValue(":username", username);
    
    if (query.exec()) {
        while (query.next()) {
            groups.append(query.value(0).toString());
        }
    }
    return groups;
}

bool DatabaseManager::addGroupMember(const QString& groupName, const QString& username) {
    QMutexLocker locker(&m_mutex);
    
    if (getGroupMemberCount(groupName) >= 10) {
        return false; // Group is full
    }
    
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM groups WHERE group_name = :name");
    query.bindValue(":name", groupName);
    
    if (!query.exec() || !query.next()) return false;
    int groupId = query.value(0).toInt();
    
    query.prepare("INSERT INTO group_members (group_id, username) VALUES (:gid, :user)");
    query.bindValue(":gid", groupId);
    query.bindValue(":user", username);
    
    return query.exec();
}

void DatabaseManager::removeGroupMember(const QString& groupName, const QString& username) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("DELETE gm FROM group_members gm "
                  "JOIN groups g ON gm.group_id = g.id "
                  "WHERE g.group_name = :name AND gm.username = :user");
    query.bindValue(":name", groupName);
    query.bindValue(":user", username);
    query.exec();
}

QStringList DatabaseManager::getGroupMembers(const QString& groupName) {
    QMutexLocker locker(&m_mutex);
    QStringList members;
    QSqlQuery query(m_db);
    
    query.prepare("SELECT gm.username FROM group_members gm "
                  "JOIN groups g ON gm.group_id = g.id "
                  "WHERE g.group_name = :name");
    query.bindValue(":name", groupName);
    
    if (query.exec()) {
        while (query.next()) {
            members.append(query.value(0).toString());
        }
    }
    return members;
}

bool DatabaseManager::isGroupAdmin(const QString& groupName, const QString& username) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("SELECT admin_username FROM groups WHERE group_name = :name");
    query.bindValue(":name", groupName);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString() == username;
    }
    return false;
}

QString DatabaseManager::getGroupAdmin(const QString& groupName) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("SELECT admin_username FROM groups WHERE group_name = :name");
    query.bindValue(":name", groupName);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

int DatabaseManager::getGroupMemberCount(const QString& groupName) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("SELECT COUNT(*) FROM group_members gm "
                  "JOIN groups g ON gm.group_id = g.id "
                  "WHERE g.group_name = :name");
    query.bindValue(":name", groupName);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

void DatabaseManager::savePrivateMessage(const QString& sender, const QString& recipient, const QString& content) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("INSERT INTO private_messages (sender, recipient, content) VALUES (:sender, :recipient, :content)");
    query.bindValue(":sender", sender);
    query.bindValue(":recipient", recipient);
    query.bindValue(":content", content);
    query.exec();
}

void DatabaseManager::saveGroupMessage(const QString& sender, const QString& groupName, const QString& content) {
    QMutexLocker locker(&m_mutex);
    QSqlQuery query(m_db);
    
    query.prepare("SELECT id FROM groups WHERE group_name = :name");
    query.bindValue(":name", groupName);
    
    if (!query.exec() || !query.next()) return;
    int groupId = query.value(0).toInt();
    
    query.prepare("INSERT INTO group_messages (sender, group_id, content) VALUES (:sender, :gid, :content)");
    query.bindValue(":sender", sender);
    query.bindValue(":gid", groupId);
    query.bindValue(":content", content);
    query.exec();
}

QList<ChatProtocol::Message> DatabaseManager::getPrivateMessageHistory(const QString& user1, const QString& user2, int limit) {
    QMutexLocker locker(&m_mutex);
    QList<ChatProtocol::Message> messages;
    QSqlQuery query(m_db);
    
    query.prepare("SELECT sender, recipient, content, timestamp FROM private_messages "
                  "WHERE (sender = :u1 AND recipient = :u2) OR (sender = :u2 AND recipient = :u1) "
                  "ORDER BY timestamp DESC LIMIT :limit");
    query.bindValue(":u1", user1);
    query.bindValue(":u2", user2);
    query.bindValue(":limit", limit);
    
    if (query.exec()) {
        while (query.next()) {
            ChatProtocol::Message msg;
            msg.type = ChatProtocol::MessageType::PRIVATE_MESSAGE;
            msg.sender = query.value(0).toString();
            msg.recipient = query.value(1).toString();
            msg.content = query.value(2).toString();
            msg.timestamp = query.value(3).toDateTime();
            messages.prepend(msg);
        }
    }
    return messages;
}

QList<ChatProtocol::Message> DatabaseManager::getGroupMessageHistory(const QString& groupName, int limit) {
    QMutexLocker locker(&m_mutex);
    QList<ChatProtocol::Message> messages;
    QSqlQuery query(m_db);
    
    query.prepare("SELECT gm.sender, g.group_name, gm.content, gm.timestamp "
                  "FROM group_messages gm "
                  "JOIN groups g ON gm.group_id = g.id "
                  "WHERE g.group_name = :name "
                  "ORDER BY gm.timestamp DESC LIMIT :limit");
    query.bindValue(":name", groupName);
    query.bindValue(":limit", limit);
    
    if (query.exec()) {
        while (query.next()) {
            ChatProtocol::Message msg;
            msg.type = ChatProtocol::MessageType::GROUP_MESSAGE;
            msg.sender = query.value(0).toString();
            msg.recipient = query.value(1).toString();
            msg.content = query.value(2).toString();
            msg.timestamp = query.value(3).toDateTime();
            messages.prepend(msg);
        }
    }
    return messages;
}