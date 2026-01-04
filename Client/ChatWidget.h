#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>

class NetworkManager;

class ChatWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ChatWidget(NetworkManager *networkManager, const QString& currentUser, 
                       const QString& contact, bool isGroup, QWidget *parent = nullptr);
    
    void appendMessage(const QString& sender, const QString& content, const QDateTime& timestamp);
    
private slots:
    void onSendClicked();
    void onMessageHistoryReceived(const QString& sender, const QString& recipient, 
                                  const QString& content, const QDateTime& timestamp);
    void onGroupMembersClicked();
    void onGroupMembersReceived(const QString& groupName, const QStringList& members, const QString& admin);
    
private:
    void setupUI();
    void loadMessageHistory();
    
    NetworkManager *m_networkManager;
    QString m_currentUser;
    QString m_contact;
    bool m_isGroup;
    
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_groupMembersButton;
    QLabel *m_contactLabel;
};

#endif // CHATWIDGET_H