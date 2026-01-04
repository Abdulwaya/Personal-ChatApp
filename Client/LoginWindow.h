#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

class NetworkManager;
class MainWindow;

class LoginWindow : public QWidget {
    Q_OBJECT
    
public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    
private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onAuthSuccess();
    void onAuthFailure(const QString& error);
    
private:
    void setupUI();
    void loadCredentials();
    void saveCredentials();
    
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    QCheckBox *m_rememberMeCheck;
    QLabel *m_statusLabel;
    
    NetworkManager *m_networkManager;
    MainWindow *m_mainWindow;
};

#endif // LOGINWINDOW_H