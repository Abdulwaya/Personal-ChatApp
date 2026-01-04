#include "ChatWidget.h"
#include "NetworkManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QScrollBar>

ChatWidget::ChatWidget(NetworkManager *networkManager, const QString& currentUser, 
                      const QString& contact, bool isGroup, QWidget *parent)
    : QWidget(parent), m_networkManager(networkManager), m_currentUser(currentUser),
      m_contact(contact), m_isGroup(isGroup) {
    
    setupUI();
    loadMessageHistory();
    
    connect(m_networkManager, &NetworkManager::messageHistoryReceived, 
            this, &ChatWidget::onMessageHistoryReceived);
    connect(m_networkManager, &NetworkManager::groupMembersReceived,
            this, &ChatWidget::onGroupMembersReceived);
}

void ChatWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Header - DARK THEME
    QWidget *header = new QWidget();
    header->setStyleSheet("background-color: #2A2A2A; padding: 10px;");
    header->setFixedHeight(60);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    
    m_contactLabel = new QLabel(m_contact);
    QFont headerFont = m_contactLabel->font();
    headerFont.setPointSize(14);
    headerFont.setBold(true);
    m_contactLabel->setFont(headerFont);
    m_contactLabel->setStyleSheet("color: #FFFFFF;");
    
    headerLayout->addWidget(m_contactLabel);
    headerLayout->addStretch();
    
    if (m_isGroup) {
        m_groupMembersButton = new QPushButton("Members");
        m_groupMembersButton->setStyleSheet("QPushButton { background-color: #128C7E; color: white; "
                                           "padding: 5px 15px; border-radius: 3px; }"
                                           "QPushButton:hover { background-color: #0D7A6F; }");
        headerLayout->addWidget(m_groupMembersButton);
        connect(m_groupMembersButton, &QPushButton::clicked, this, &ChatWidget::onGroupMembersClicked);
    }
    
    // Chat display - DARK THEME
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setStyleSheet("QTextEdit { background-color: #0D1418; border: none; padding: 10px; }");
    
    // Input area - DARK THEME
    QWidget *inputArea = new QWidget();
    inputArea->setStyleSheet("background-color: #1E1E1E; padding: 10px;");
    
    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(10, 10, 10, 10);
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Type a message");
    m_messageInput->setStyleSheet("QLineEdit { padding: 10px; border: 1px solid #3A3A3A; "
                                 "border-radius: 20px; background-color: #2A2A2A; color: #FFFFFF; }"
                                 "QLineEdit::placeholder { color: #8696A0; }");
    
    m_sendButton = new QPushButton("Send");
    m_sendButton->setStyleSheet("QPushButton { background-color: #25D366; color: white; "
                               "padding: 10px 20px; border-radius: 20px; font-weight: bold; }"
                               "QPushButton:hover { background-color: #20BA5A; }");
    
    inputLayout->addWidget(m_messageInput);
    inputLayout->addWidget(m_sendButton);
    
    mainLayout->addWidget(header);
    mainLayout->addWidget(m_chatDisplay, 1);
    mainLayout->addWidget(inputArea);
    
    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatWidget::onSendClicked);
}

void ChatWidget::loadMessageHistory() {
    m_networkManager->requestMessageHistory(m_contact, m_isGroup);
}

void ChatWidget::onSendClicked() {
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) return;
    
    if (m_isGroup) {
        m_networkManager->sendGroupMessage(m_contact, message);
    } else {
        m_networkManager->sendPrivateMessage(m_contact, message);
    }
    
    appendMessage(m_currentUser, message, QDateTime::currentDateTime());
    m_messageInput->clear();
}

void ChatWidget::appendMessage(const QString& sender, const QString& content, const QDateTime& timestamp) {
    bool isSentByMe = (sender == m_currentUser);
    
    QString alignment = isSentByMe ? "right" : "left";
    QString bgColor = isSentByMe ? "#005C4B" : "#1E2A2F";
    QString senderName = m_isGroup && !isSentByMe ? sender + ": " : "";
    QString timeStr = timestamp.toString("hh:mm");
    
    QString messageHtml = QString(
        "<div style='text-align: %1; margin: 5px;'>"
        "  <div style='display: inline-block; background-color: %2; padding: 8px 12px; "
        "             border-radius: 10px; max-width: 70%; text-align: left;'>"
        "    <span style='font-weight: bold; color: #25D366;'>%3</span>"
        "    <span style='color: #E9EDEF;'>%4</span><br>"
        "    <span style='color: #8696A0; font-size: 10px;'>%5</span>"
        "  </div>"
        "</div>"
    ).arg(alignment, bgColor, senderName, content, timeStr);
    
    m_chatDisplay->append(messageHtml);
    m_chatDisplay->verticalScrollBar()->setValue(m_chatDisplay->verticalScrollBar()->maximum());
}

void ChatWidget::onMessageHistoryReceived(const QString& sender, const QString& recipient, 
                                         const QString& content, const QDateTime& timestamp) {
    if (m_isGroup && recipient == m_contact) {
        appendMessage(sender, content, timestamp);
    } else if (!m_isGroup && (sender == m_contact || recipient == m_contact)) {
        appendMessage(sender, content, timestamp);
    }
}

void ChatWidget::onGroupMembersClicked() {
    m_networkManager->requestGroupMembers(m_contact);
}

void ChatWidget::onGroupMembersReceived(const QString& groupName, const QStringList& members, const QString& admin) {
    if (groupName != m_contact) return;
    
    QString membersList = "Group Admin: " + admin + "\n\nMembers:\n" + members.join("\n");
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Group Members");
    msgBox.setText(membersList);
    
    if (admin == m_currentUser && members.size() > 1) {
        msgBox.addButton("Kick Member", QMessageBox::ActionRole);
        msgBox.addButton("Leave Group", QMessageBox::DestructiveRole);
        msgBox.addButton("Close", QMessageBox::RejectRole);
    } else {
        msgBox.addButton("Leave Group", QMessageBox::DestructiveRole);
        msgBox.addButton("Close", QMessageBox::RejectRole);
    }
    
    int ret = msgBox.exec();
    
    if (ret == 0 && admin == m_currentUser) {
        bool ok;
        QString member = QInputDialog::getItem(this, "Kick Member", "Select member:", 
                                               members, 0, false, &ok);
        if (ok && member != admin) {
            m_networkManager->kickMember(m_contact, member);
            QMessageBox::information(this, "Success", member + " has been removed from the group.");
        }
    } else if ((ret == 1 && admin == m_currentUser) || (ret == 0 && admin != m_currentUser)) {
        m_networkManager->leaveGroup(m_contact);
        QMessageBox::information(this, "Success", "You have left the group.");
    }
}