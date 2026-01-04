#include "LoginWindow.h"
#include "NetworkManager.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSettings>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent), m_mainWindow(nullptr) {
    setupUI();
    
    m_networkManager = new NetworkManager(this);
    connect(m_networkManager, &NetworkManager::authSuccess, this, &LoginWindow::onAuthSuccess);
    connect(m_networkManager, &NetworkManager::authFailure, this, &LoginWindow::onAuthFailure);
    
    loadCredentials();
}

LoginWindow::~LoginWindow() {
    if (m_mainWindow) {
        delete m_mainWindow;
    }
}

void LoginWindow::setupUI() {
    setWindowTitle("Chat App - Login");
    setFixedSize(400, 350);
    
    // Dark theme for login window
    setStyleSheet("QWidget { background-color: #1E1E1E; }");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // Title
    QLabel *titleLabel = new QLabel("Chat Application");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #25D366;");
    
    // Username
    QLabel *usernameLabel = new QLabel("Username:");
    usernameLabel->setStyleSheet("color: #E9EDEF;");
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("Enter username");
    m_usernameEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #3A3A3A; border-radius: 5px; "
                                  "background-color: #2A2A2A; color: #FFFFFF; }"
                                  "QLineEdit::placeholder { color: #8696A0; }");
    
    // Password
    QLabel *passwordLabel = new QLabel("Password:");
    passwordLabel->setStyleSheet("color: #E9EDEF;");
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Enter password");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #3A3A3A; border-radius: 5px; "
                                  "background-color: #2A2A2A; color: #FFFFFF; }"
                                  "QLineEdit::placeholder { color: #8696A0; }");
    
    // Remember me
    m_rememberMeCheck = new QCheckBox("Remember me");
    m_rememberMeCheck->setStyleSheet("QCheckBox { color: #E9EDEF; }");
    
    // Buttons
    m_loginButton = new QPushButton("Login");
    m_loginButton->setStyleSheet("QPushButton { background-color: #128C7E; color: white; padding: 10px; "
                                 "border-radius: 5px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #0D7A6F; }");
    
    m_registerButton = new QPushButton("Register");
    m_registerButton->setStyleSheet("QPushButton { background-color: #25D366; color: white; padding: 10px; "
                                    "border-radius: 5px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #20BA5A; }");
    
    // Status
    m_statusLabel = new QLabel();
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #FF6B6B;");
    
    // Layout assembly
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(usernameLabel);
    mainLayout->addWidget(m_usernameEdit);
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(m_passwordEdit);
    mainLayout->addWidget(m_rememberMeCheck);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_loginButton);
    mainLayout->addWidget(m_registerButton);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addStretch();
    
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
}

void LoginWindow::onLoginClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Please fill in all fields");
        return;
    }
    
    m_statusLabel->setText("Connecting...");
    m_networkManager->connectToServer("127.0.0.1", 12345);
    m_networkManager->login(username, password);
}

void LoginWindow::onRegisterClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Please fill in all fields");
        return;
    }
    
    m_statusLabel->setText("Connecting...");
    m_networkManager->connectToServer("127.0.0.1", 12345);
    m_networkManager->registerUser(username, password);
}

void LoginWindow::onAuthSuccess() {
    m_statusLabel->setStyleSheet("color: green;");
    m_statusLabel->setText("Login successful!");
    
    if (m_rememberMeCheck->isChecked()) {
        saveCredentials();
    }
    
    m_mainWindow = new MainWindow(m_networkManager, m_usernameEdit->text());
    m_mainWindow->show();
    this->hide();
}

void LoginWindow::onAuthFailure(const QString& error) {
    m_statusLabel->setStyleSheet("color: red;");
    m_statusLabel->setText(error);
}

void LoginWindow::loadCredentials() {
    QSettings settings("ChatApp", "Login");
    if (settings.value("rememberMe", false).toBool()) {
        m_usernameEdit->setText(settings.value("username").toString());
        m_passwordEdit->setText(settings.value("password").toString());
        m_rememberMeCheck->setChecked(true);
    }
}

void LoginWindow::saveCredentials() {
    QSettings settings("ChatApp", "Login");
    settings.setValue("rememberMe", m_rememberMeCheck->isChecked());
    settings.setValue("username", m_usernameEdit->text());
    settings.setValue("password", m_passwordEdit->text());
}