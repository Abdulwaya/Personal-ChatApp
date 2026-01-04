#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QThread>
#include <QTcpSocket>
#include "Protocol.h"

class ChatServer;
class DatabaseManager;

class ClientHandler : public QThread {
    Q_OBJECT
    
public:
    ClientHandler(qintptr socketDescriptor, ChatServer *server, DatabaseManager *db);
    ~ClientHandler();
    
    void sendMessage(const ChatProtocol::Message& msg);
    QString getUsername() const { return m_username; }
    
signals:
    void disconnected(const QString& username);
    
protected:
    void run() override;
    
private slots:
    void onReadyRead();
    void onSocketDisconnected();
    
private:
    void handleMessage(const ChatProtocol::Message& msg);
    void handleRegister(const ChatProtocol::Message& msg);
    void handleLogin(const ChatProtocol::Message& msg);
    void handlePrivateMessage(const ChatProtocol::Message& msg);
    void handleCreateGroup(const ChatProtocol::Message& msg);
    void handleGroupMessage(const ChatProtocol::Message& msg);
    void handleGetUsers(const ChatProtocol::Message& msg);
    void handleGetGroups(const ChatProtocol::Message& msg);
    void handleMessageHistory(const ChatProtocol::Message& msg);
    void handleLeaveGroup(const ChatProtocol::Message& msg);
    void handleKickMember(const ChatProtocol::Message& msg);
    void handleGroupMembersRequest(const ChatProtocol::Message& msg);
    
    qintptr m_socketDescriptor;
    QTcpSocket *m_socket;
    ChatServer *m_server;
    DatabaseManager *m_database;
    QString m_username;
    bool m_authenticated;
};

#endif // CLIENTHANDLER_H