#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>

class NetworkManager;
class ChatWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(NetworkManager *networkManager, const QString& username, QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void onContactSelected(QListWidgetItem *item);
    void onNewChatClicked();
    void onNewGroupClicked();
    void onUsersListReceived(const QStringList& users);
    void onGroupsListReceived(const QStringList& groups);
    void onGroupCreated(const QString& groupName);
    void onPrivateMessageReceived(const QString& sender, const QString& content, const QDateTime& timestamp);
    void onGroupMessageReceived(const QString& sender, const QString& groupName, const QString& content, const QDateTime& timestamp);
    
private:
    void setupUI();
    void loadContacts();
    ChatWidget* getChatWidget(const QString& contact, bool isGroup);
    
    NetworkManager *m_networkManager;
    QString m_username;
    QStringList m_allUsers;  // Store all users
    
    QListWidget *m_contactsList;
    QStackedWidget *m_chatStack;
    QPushButton *m_newChatButton;
    QPushButton *m_newGroupButton;
    QLabel *m_usernameLabel;
    
    QMap<QString, ChatWidget*> m_chatWidgets;
    QStringList m_groups;
};

#endif // MAINWINDOW_H