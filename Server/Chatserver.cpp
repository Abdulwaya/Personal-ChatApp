#include "ChatServer.h"
#include "ClientHandler.h"
#include <QDebug>

ChatServer::ChatServer(QObject *parent) : QTcpServer(parent) {
    if (!m_database.connect()) {
        qDebug() << "Failed to connect to database!";
    }
}

ChatServer::~ChatServer() {
    QMutexLocker locker(&m_clientsMutex);
    qDeleteAll(m_clients);
}

bool ChatServer::startServer(quint16 port) {
    return listen(QHostAddress::Any, port);
}

void ChatServer::incomingConnection(qintptr socketDescriptor) {
    qDebug() << "New connection incoming...";
    ClientHandler *handler = new ClientHandler(socketDescriptor, this, &m_database);
    
    connect(handler, &ClientHandler::disconnected, this, &ChatServer::onClientDisconnected);
    
    handler->start(); // Start the thread
}

void ChatServer::onClientDisconnected(const QString& username) {
    QMutexLocker locker(&m_clientsMutex);
    if (m_clients.contains(username)) {
        m_clients.remove(username);
        m_database.setUserOnlineStatus(username, false);
        qDebug() << "User disconnected:" << username;
    }
}

void ChatServer::broadcastToUser(const QString& username, const ChatProtocol::Message& msg) {
    QMutexLocker locker(&m_clientsMutex);
    if (m_clients.contains(username)) {
        m_clients[username]->sendMessage(msg);
    }
}

void ChatServer::broadcastToGroup(const QString& groupName, const ChatProtocol::Message& msg) {
    QStringList members = m_database.getGroupMembers(groupName);
    QMutexLocker locker(&m_clientsMutex);
    
    for (const QString& member : members) {
        if (m_clients.contains(member)) {
            m_clients[member]->sendMessage(msg);
        }
    }
}