#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QMutex>
#include "Protocol.h"

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();
    
    bool connect();
    void disconnect();
    
    // User management
    bool registerUser(const QString& username, const QString& password);
    bool loginUser(const QString& username, const QString& password);
    QStringList getAllUsers();
    void setUserOnlineStatus(const QString& username, bool online);
    
    // Group management
    bool createGroup(const QString& groupName, const QString& adminUsername);
    QStringList getUserGroups(const QString& username);
    bool addGroupMember(const QString& groupName, const QString& username);
    void removeGroupMember(const QString& groupName, const QString& username);
    QStringList getGroupMembers(const QString& groupName);
    bool isGroupAdmin(const QString& groupName, const QString& username);
    QString getGroupAdmin(const QString& groupName);
    int getGroupMemberCount(const QString& groupName);
    
    // Message management
    void savePrivateMessage(const QString& sender, const QString& recipient, const QString& content);
    void saveGroupMessage(const QString& sender, const QString& groupName, const QString& content);
    QList<ChatProtocol::Message> getPrivateMessageHistory(const QString& user1, const QString& user2, int limit);
    QList<ChatProtocol::Message> getGroupMessageHistory(const QString& groupName, int limit);
    
private:
    void createTables();
    
    QSqlDatabase m_db;
    QMutex m_mutex;
};

#endif // DATABASEMANAGER_H