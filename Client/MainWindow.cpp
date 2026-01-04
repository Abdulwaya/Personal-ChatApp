#include "MainWindow.h"
#include "NetworkManager.h"
#include "ChatWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QSplitter>

MainWindow::MainWindow(NetworkManager *networkManager, const QString& username, QWidget *parent)
    : QMainWindow(parent), m_networkManager(networkManager), m_username(username) {
    
    setupUI();
    loadContacts();
    
    connect(m_networkManager, &NetworkManager::usersListReceived, this, &MainWindow::onUsersListReceived);
    connect(m_networkManager, &NetworkManager::groupsListReceived, this, &MainWindow::onGroupsListReceived);
    connect(m_networkManager, &NetworkManager::groupCreated, this, &MainWindow::onGroupCreated);
    connect(m_networkManager, &NetworkManager::privateMessageReceived, this, &MainWindow::onPrivateMessageReceived);
    connect(m_networkManager, &NetworkManager::groupMessageReceived, this, &MainWindow::onGroupMessageReceived);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("Chat App");
    resize(1000, 600);
    
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Left sidebar
    QWidget *sidebar = new QWidget();
    sidebar->setStyleSheet("background-color: #EDEDED;");
    sidebar->setMaximumWidth(350);
    sidebar->setMinimumWidth(300);
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(0);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    
    // User header
    QWidget *userHeader = new QWidget();
    userHeader->setStyleSheet("background-color: #EDEDED; padding: 10px;");
    QHBoxLayout *userHeaderLayout = new QHBoxLayout(userHeader);
    
    m_usernameLabel = new QLabel(m_username);
    QFont usernameFont = m_usernameLabel->font();
    usernameFont.setPointSize(12);
    usernameFont.setBold(true);
    m_usernameLabel->setFont(usernameFont);
    
    userHeaderLayout->addWidget(m_usernameLabel);
    userHeaderLayout->addStretch();
    
    // Action buttons
    QWidget *actionsWidget = new QWidget();
    actionsWidget->setStyleSheet("background-color: #EDEDED;");
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(10, 5, 10, 10);
    
    m_newChatButton = new QPushButton("New Chat");
    m_newChatButton->setStyleSheet("QPushButton { background-color: #25D366; color: white; padding: 8px 15px; "
                                   "border-radius: 5px; font-weight: bold; }"
                                   "QPushButton:hover { background-color: #20BA5A; }");
    
    m_newGroupButton = new QPushButton("New Group");
    m_newGroupButton->setStyleSheet("QPushButton { background-color: #075E54; color: white; padding: 8px 15px; "
                                    "border-radius: 5px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #128C7E; }");
    
    actionsLayout->addWidget(m_newChatButton);
    actionsLayout->addWidget(m_newGroupButton);
    
    // Contacts list
    m_contactsList = new QListWidget();
    m_contactsList->setStyleSheet("QListWidget { background-color: white; border: none; }"
                                 "QListWidget::item { padding: 12px; border-bottom: 1px solid #f0f0f0; }"
                                 "QListWidget::item:selected { background-color: #EDEDED; }");
    
    sidebarLayout->addWidget(userHeader);
    sidebarLayout->addWidget(actionsWidget);
    sidebarLayout->addWidget(m_contactsList);
    
    // Chat area
    m_chatStack = new QStackedWidget();
    m_chatStack->setStyleSheet("background-color: #E5DDD5;");
    
    QLabel *welcomeLabel = new QLabel("Select a chat to start messaging");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #888; font-size: 16px;");
    m_chatStack->addWidget(welcomeLabel);
    
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(m_chatStack, 1);
    
    connect(m_contactsList, &QListWidget::itemClicked, this, &MainWindow::onContactSelected);
    connect(m_newChatButton, &QPushButton::clicked, this, &MainWindow::onNewChatClicked);
    connect(m_newGroupButton, &QPushButton::clicked, this, &MainWindow::onNewGroupClicked);
}

void MainWindow::loadContacts() {
    m_networkManager->requestUsers();
    m_networkManager->requestGroups();
}

void MainWindow::onContactSelected(QListWidgetItem *item) {
    QString contact = item->text();
    bool isGroup = m_groups.contains(contact);
    
    ChatWidget *chatWidget = getChatWidget(contact, isGroup);
    m_chatStack->setCurrentWidget(chatWidget);
}

ChatWidget* MainWindow::getChatWidget(const QString& contact, bool isGroup) {
    if (m_chatWidgets.contains(contact)) {
        return m_chatWidgets[contact];
    }
    
    ChatWidget *chatWidget = new ChatWidget(m_networkManager, m_username, contact, isGroup);
    m_chatWidgets[contact] = chatWidget;
    m_chatStack->addWidget(chatWidget);
    
    return chatWidget;
}

void MainWindow::onNewChatClicked() {
    m_networkManager->requestUsers();
}

void MainWindow::onNewGroupClicked() {
    bool ok;
    QString groupName = QInputDialog::getText(this, "Create Group", 
                                             "Enter group name:", 
                                             QLineEdit::Normal, "", &ok);
    
    if (ok && !groupName.isEmpty()) {
        m_networkManager->createGroup(groupName);
    }
}

void MainWindow::onUsersListReceived(const QStringList& users) {
    m_contactsList->clear();
    
    // Add groups first
    for (const QString& group : m_groups) {
        QListWidgetItem *item = new QListWidgetItem(group);
        item->setIcon(QIcon());
        m_contactsList->addItem(item);
    }
    
    // Add users
    for (const QString& user : users) {
        if (user != m_username) {
            QListWidgetItem *item = new QListWidgetItem(user);
            m_contactsList->addItem(item);
        }
    }
}

void MainWindow::onGroupsListReceived(const QStringList& groups) {
    m_groups = groups;
    m_networkManager->requestUsers();
}

void MainWindow::onGroupCreated(const QString& groupName) {
    QMessageBox::information(this, "Success", "Group created: " + groupName);
    m_groups.append(groupName);
    loadContacts();
}

void MainWindow::onPrivateMessageReceived(const QString& sender, const QString& content, const QDateTime& timestamp) {
    if (sender == m_username) return;
    
    ChatWidget *chatWidget = getChatWidget(sender, false);
    chatWidget->appendMessage(sender, content, timestamp);
}

void MainWindow::onGroupMessageReceived(const QString& sender, const QString& groupName, 
                                       const QString& content, const QDateTime& timestamp) {
    ChatWidget *chatWidget = getChatWidget(groupName, true);
    chatWidget->appendMessage(sender, content, timestamp);
}