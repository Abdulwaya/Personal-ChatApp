#include "NetworkManager.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent) 
    : QObject(parent), m_socket(new QTcpSocket(this)), m_expectedSize(0) {
    
    connect(m_socket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkManager::onError);
}

NetworkManager::~NetworkManager() {
    if (m_socket->isOpen()) {
        m_socket->close();
    }
}

void NetworkManager::connectToServer(const QString& host, quint16 port) {
    m_socket->connectToHost(host, port);
}

void NetworkManager::sendMessage(const ChatProtocol::Message& msg) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    
    QByteArray data = msg.serialize();
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << quint32(data.size());
    block.append(data);
    
    m_socket->write(block);
    m_socket->flush();
}

void NetworkManager::registerUser(const QString& username, const QString& password) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::REGISTER;
    msg.sender = username;
    msg.content = password;
    sendMessage(msg);
}

void NetworkManager::login(const QString& username, const QString& password) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::LOGIN;
    msg.sender = username;
    msg.content = password;
    sendMessage(msg);
}

void NetworkManager::sendPrivateMessage(const QString& recipient, const QString& content) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::PRIVATE_MESSAGE;
    msg.recipient = recipient;
    msg.content = content;
    sendMessage(msg);
}

void NetworkManager::sendGroupMessage(const QString& groupName, const QString& content) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::GROUP_MESSAGE;
    msg.recipient = groupName;
    msg.content = content;
    sendMessage(msg);
}

void NetworkManager::createGroup(const QString& groupName) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::CREATE_GROUP;
    msg.content = groupName;
    sendMessage(msg);
}

void NetworkManager::requestUsers() {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::GET_USERS;
    sendMessage(msg);
}

void NetworkManager::requestGroups() {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::GET_GROUPS;
    sendMessage(msg);
}

void NetworkManager::requestMessageHistory(const QString& recipient, bool isGroup) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::MESSAGE_HISTORY_REQUEST;
    msg.recipient = recipient;
    if (isGroup) {
        msg.content = "GROUP:" + recipient;
    }
    sendMessage(msg);
}

void NetworkManager::leaveGroup(const QString& groupName) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::LEAVE_GROUP;
    msg.content = groupName;
    sendMessage(msg);
}

void NetworkManager::kickMember(const QString& groupName, const QString& member) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::KICK_MEMBER;
    msg.recipient = groupName;
    msg.content = member;
    sendMessage(msg);
}

void NetworkManager::requestGroupMembers(const QString& groupName) {
    ChatProtocol::Message msg;
    msg.type = ChatProtocol::MessageType::GROUP_MEMBERS_REQUEST;
    msg.content = groupName;
    sendMessage(msg);
}

void NetworkManager::onConnected() {
    qDebug() << "Connected to server";
    emit connected();
}

void NetworkManager::onDisconnected() {
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void NetworkManager::onReadyRead() {
    while (m_socket->bytesAvailable() > 0) {
        if (m_expectedSize == 0) {
            if (m_socket->bytesAvailable() < sizeof(quint32)) return;
            QDataStream in(m_socket);
            in >> m_expectedSize;
        }
        
        m_buffer.append(m_socket->read(m_expectedSize - m_buffer.size()));
        
        if (m_buffer.size() == m_expectedSize) {
            ChatProtocol::Message msg = ChatProtocol::Message::deserialize(m_buffer);
            handleMessage(msg);
            m_buffer.clear();
            m_expectedSize = 0;
        }
    }
}

void NetworkManager::onError(QAbstractSocket::SocketError error) {
    qDebug() << "Socket error:" << m_socket->errorString();
    emit errorOccurred(m_socket->errorString());
}

void NetworkManager::handleMessage(const ChatProtocol::Message& msg) {
    switch (msg.type) {
        case ChatProtocol::MessageType::AUTH_SUCCESS:
            emit authSuccess();
            break;
            
        case ChatProtocol::MessageType::AUTH_FAILURE:
            emit authFailure(msg.content);
            break;
            
        case ChatProtocol::MessageType::PRIVATE_MESSAGE:
            emit privateMessageReceived(msg.sender, msg.content, msg.timestamp);
            break;
            
        case ChatProtocol::MessageType::GROUP_MESSAGE:
            emit groupMessageReceived(msg.sender, msg.recipient, msg.content, msg.timestamp);
            break;
            
        case ChatProtocol::MessageType::USERS_LIST:
            emit usersListReceived(msg.content.split(",", Qt::SkipEmptyParts));
            break;
            
        case ChatProtocol::MessageType::GROUPS_LIST:
            emit groupsListReceived(msg.content.split(",", Qt::SkipEmptyParts));
            break;
            
        case ChatProtocol::MessageType::GROUP_CREATED:
            emit groupCreated(msg.content);
            break;
            
        case ChatProtocol::MessageType::MESSAGE_HISTORY_RESPONSE:
            emit messageHistoryReceived(msg.sender, msg.recipient, msg.content, msg.timestamp);
            break;
            
        case ChatProtocol::MessageType::GROUP_MEMBERS_RESPONSE:
            emit groupMembersReceived(msg.recipient, msg.content.split(",", Qt::SkipEmptyParts), msg.sender);
            break;
            
        case ChatProtocol::MessageType::ERROR_MSG:
            emit errorOccurred(msg.content);
            break;
            
        default:
            break;
    }
}