#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include "Protocol.h"

class NetworkManager : public QObject {
    Q_OBJECT
    
public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    
    void connectToServer(const QString& host, quint16 port);
    void registerUser(const QString& username, const QString& password);
    void login(const QString& username, const QString& password);
    void sendPrivateMessage(const QString& recipient, const QString& content);
    void sendGroupMessage(const QString& groupName, const QString& content);
    void createGroup(const QString& groupName);
    void requestUsers();
    void requestGroups();
    void requestMessageHistory(const QString& recipient, bool isGroup = false);
    void leaveGroup(const QString& groupName);
    void kickMember(const QString& groupName, const QString& member);
    void requestGroupMembers(const QString& groupName);
    
signals:
    void connected();
    void disconnected();
    void authSuccess();
    void authFailure(const QString& error);
    void privateMessageReceived(const QString& sender, const QString& content, const QDateTime& timestamp);
    void groupMessageReceived(const QString& sender, const QString& groupName, const QString& content, const QDateTime& timestamp);
    void usersListReceived(const QStringList& users);
    void groupsListReceived(const QStringList& groups);
    void groupCreated(const QString& groupName);
    void messageHistoryReceived(const QString& sender, const QString& recipient, const QString& content, const QDateTime& timestamp);
    void groupMembersReceived(const QString& groupName, const QStringList& members, const QString& admin);
    void errorOccurred(const QString& error);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    
private:
    void sendMessage(const ChatProtocol::Message& msg);
    void handleMessage(const ChatProtocol::Message& msg);
    
    QTcpSocket *m_socket;
    quint32 m_expectedSize;
    QByteArray m_buffer;
};

#endif // NETWORKMANAGER_H