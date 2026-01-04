#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QMutex>
#include "Protocol.h"
#include "DatabaseManager.h"

class ClientHandler;

class ChatServer : public QTcpServer {
    Q_OBJECT
    
public:
    explicit ChatServer(QObject *parent = nullptr);
    ~ChatServer();
    
    bool startServer(quint16 port);
    void broadcastToUser(const QString& username, const ChatProtocol::Message& msg);
    void broadcastToGroup(const QString& groupName, const ChatProtocol::Message& msg);
    
protected:
    void incomingConnection(qintptr socketDescriptor) override;
    
private slots:
    void onClientDisconnected(const QString& username);
    
private:
    QMap<QString, ClientHandler*> m_clients; // username -> handler
    QMutex m_clientsMutex;
    DatabaseManager m_database;
    
    friend class ClientHandler;
};

#endif // CHATSERVER_H