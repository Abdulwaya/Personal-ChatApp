#include "ClientHandler.h"
#include "ChatServer.h"
#include "DatabaseManager.h"
#include <QDebug>

ClientHandler::ClientHandler(qintptr socketDescriptor, ChatServer *server, DatabaseManager *db)
    : m_socketDescriptor(socketDescriptor), m_socket(nullptr), m_server(server),
      m_database(db), m_authenticated(false) {}

ClientHandler::~ClientHandler() {
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
    }
}

void ClientHandler::run() {
    m_socket = new QTcpSocket();
    
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Failed to set socket descriptor";
        return;
    }
    
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead, Qt::DirectConnection);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onSocketDisconnected, Qt::DirectConnection);
    
    exec(); // Event loop for this thread
}

void ClientHandler::sendMessage(const ChatProtocol::Message& msg) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) return;
    
    QByteArray data = msg.serialize();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << quint32(data.size());
    block.append(data);
    
    m_socket->write(block);
    m_socket->flush();
}

void ClientHandler::onReadyRead() {
    static quint32 expectedSize = 0;
    static QByteArray buffer;
    
    while (m_socket->bytesAvailable() > 0) {
        if (expectedSize == 0) {
            if (m_socket->bytesAvailable() < sizeof(quint32)) return;
            QDataStream in(m_socket);
            in >> expectedSize;
        }
        
        buffer.append(m_socket->read(expectedSize - buffer.size()));
        
        if (buffer.size() == expectedSize) {
            ChatProtocol::Message msg = ChatProtocol::Message::deserialize(buffer);
            handleMessage(msg);
            buffer.clear();
            expectedSize = 0;
        }
    }
}

void ClientHandler::onSocketDisconnected() {
    emit disconnected(m_username);
    quit();
}

void ClientHandler::handleMessage(const ChatProtocol::Message& msg) {
    switch (msg.type) {
        case ChatProtocol::MessageType::REGISTER:
            handleRegister(msg);
            break;
        case ChatProtocol::MessageType::LOGIN:
            handleLogin(msg);
            break;
        case ChatProtocol::MessageType::PRIVATE_MESSAGE:
            handlePrivateMessage(msg);
            break;
        case ChatProtocol::MessageType::CREATE_GROUP:
            handleCreateGroup(msg);
            break;
        case ChatProtocol::MessageType::GROUP_MESSAGE:
            handleGroupMessage(msg);
            break;
        case ChatProtocol::MessageType::GET_USERS:
            handleGetUsers(msg);
            break;
        case ChatProtocol::MessageType::GET_GROUPS:
            handleGetGroups(msg);
            break;
        case ChatProtocol::MessageType::MESSAGE_HISTORY_REQUEST:
            handleMessageHistory(msg);
            break;
        case ChatProtocol::MessageType::LEAVE_GROUP:
            handleLeaveGroup(msg);
            break;
        case ChatProtocol::MessageType::KICK_MEMBER:
            handleKickMember(msg);
            break;
        case ChatProtocol::MessageType::GROUP_MEMBERS_REQUEST:
            handleGroupMembersRequest(msg);
            break;
        default:
            qDebug() << "Unknown message type";
    }
}

void ClientHandler::handleRegister(const ChatProtocol::Message& msg) {
    ChatProtocol::Message response;
    
    if (m_database->registerUser(msg.sender, msg.content)) {
        response.type = ChatProtocol::MessageType::AUTH_SUCCESS;
        response.content = "Registration successful";
    } else {
        response.type = ChatProtocol::MessageType::AUTH_FAILURE;
        response.content = "Username already exists";
    }
    
    sendMessage(response);
}

void ClientHandler::handleLogin(const ChatProtocol::Message& msg) {
    ChatProtocol::Message response;
    
    if (m_database->loginUser(msg.sender, msg.content)) {
        m_username = msg.sender;
        m_authenticated = true;
        m_database->setUserOnlineStatus(m_username, true);
        
        QMutexLocker locker(&m_server->m_clientsMutex);
        m_server->m_clients[m_username] = this;
        
        response.type = ChatProtocol::MessageType::AUTH_SUCCESS;
        response.content = "Login successful";
        qDebug() << "User logged in:" << m_username;
    } else {
        response.type = ChatProtocol::MessageType::AUTH_FAILURE;
        response.content = "Invalid credentials";
    }
    
    sendMessage(response);
}

void ClientHandler::handlePrivateMessage(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    m_database->savePrivateMessage(msg.sender, msg.recipient, msg.content);
    m_server->broadcastToUser(msg.recipient, msg);
}

void ClientHandler::handleCreateGroup(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    ChatProtocol::Message response;
    
    if (m_database->createGroup(msg.content, m_username)) {
        response.type = ChatProtocol::MessageType::GROUP_CREATED;
        response.content = msg.content;
    } else {
        response.type = ChatProtocol::MessageType::ERROR_MSG;
        response.content = "Group already exists";
    }
    
    sendMessage(response);
}

void ClientHandler::handleGroupMessage(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    m_database->saveGroupMessage(msg.sender, msg.recipient, msg.content);
    m_server->broadcastToGroup(msg.recipient, msg);
}

void ClientHandler::handleGetUsers(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    QStringList users = m_database->getAllUsers();
    ChatProtocol::Message response;
    response.type = ChatProtocol::MessageType::USERS_LIST;
    response.content = users.join(",");
    
    sendMessage(response);
}

void ClientHandler::handleGetGroups(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    QStringList groups = m_database->getUserGroups(m_username);
    ChatProtocol::Message response;
    response.type = ChatProtocol::MessageType::GROUPS_LIST;
    response.content = groups.join(",");
    
    sendMessage(response);
}

void ClientHandler::handleMessageHistory(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    QList<ChatProtocol::Message> history;
    
    if (msg.content.startsWith("GROUP:")) {
        QString groupName = msg.content.mid(6);
        history = m_database->getGroupMessageHistory(groupName, 100);
    } else {
        history = m_database->getPrivateMessageHistory(m_username, msg.recipient, 100);
    }
    
    for (const auto& histMsg : history) {
        ChatProtocol::Message response;
        response.type = ChatProtocol::MessageType::MESSAGE_HISTORY_RESPONSE;
        response.sender = histMsg.sender;
        response.recipient = histMsg.recipient;
        response.content = histMsg.content;
        response.timestamp = histMsg.timestamp;
        sendMessage(response);
    }
}

void ClientHandler::handleLeaveGroup(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    m_database->removeGroupMember(msg.content, m_username);
    
    ChatProtocol::Message response;
    response.type = ChatProtocol::MessageType::SUCCESS_MSG;
    response.content = "Left group: " + msg.content;
    sendMessage(response);
}

void ClientHandler::handleKickMember(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    QString groupName = msg.recipient;
    QString memberToKick = msg.content;
    
    if (m_database->isGroupAdmin(groupName, m_username)) {
        m_database->removeGroupMember(groupName, memberToKick);
        
        ChatProtocol::Message notification;
        notification.type = ChatProtocol::MessageType::SUCCESS_MSG;
        notification.content = "You were removed from " + groupName;
        m_server->broadcastToUser(memberToKick, notification);
    }
}

void ClientHandler::handleGroupMembersRequest(const ChatProtocol::Message& msg) {
    if (!m_authenticated) return;
    
    QStringList members = m_database->getGroupMembers(msg.content);
    QString adminUsername = m_database->getGroupAdmin(msg.content);
    
    ChatProtocol::Message response;
    response.type = ChatProtocol::MessageType::GROUP_MEMBERS_RESPONSE;
    response.recipient = msg.content;
    response.content = members.join(",");
    response.sender = adminUsername;
    
    sendMessage(response);
}