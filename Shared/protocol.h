#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QDataStream>
#include <QDateTime>
#include <QIODevice>
#include <QByteArray>

namespace ChatProtocol {

enum class MessageType {
    // Authentication
    REGISTER,
    LOGIN,
    LOGOUT,
    AUTH_SUCCESS,
    AUTH_FAILURE,
    
    // User Management
    GET_USERS,
    USERS_LIST,
    
    // Private Messages
    PRIVATE_MESSAGE,
    MESSAGE_HISTORY_REQUEST,
    MESSAGE_HISTORY_RESPONSE,
    
    // Group Chat
    CREATE_GROUP,
    GROUP_CREATED,
    JOIN_GROUP,
    LEAVE_GROUP,
    GROUP_MESSAGE,
    GET_GROUPS,
    GROUPS_LIST,
    GROUP_MEMBERS_REQUEST,
    GROUP_MEMBERS_RESPONSE,
    KICK_MEMBER,
    
    // Status
    ERROR_MSG,
    SUCCESS_MSG
};

struct Message {
    MessageType type;
    QString sender;
    QString recipient; // username for private, groupname for group
    QString content;
    QDateTime timestamp;
    int messageId = 0;
    
    Message() : type(MessageType::ERROR_MSG), timestamp(QDateTime::currentDateTime()) {}
    
    QByteArray serialize() const {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << static_cast<int>(type);
        stream << sender;
        stream << recipient;
        stream << content;
        stream << timestamp;
        stream << messageId;
        return data;
    }
    
    static Message deserialize(const QByteArray& data) {
        Message msg;
        QDataStream stream(data);
        int typeInt;
        stream >> typeInt;
        msg.type = static_cast<MessageType>(typeInt);
        stream >> msg.sender;
        stream >> msg.recipient;
        stream >> msg.content;
        stream >> msg.timestamp;
        stream >> msg.messageId;
        return msg;
    }
};

struct User {
    int userId = 0;
    QString username;
    QString password;
    bool isOnline = false;
    
    User() = default;
    User(const QString& name, const QString& pass) 
        : username(name), password(pass) {}
};

struct Group {
    int groupId = 0;
    QString groupName;
    QString adminUsername;
    QStringList members;
    
    Group() = default;
    Group(const QString& name, const QString& admin) 
        : groupName(name), adminUsername(admin) {
        members.append(admin);
    }
};

} // namespace ChatProtocol

#endif // PROTOCOL_H