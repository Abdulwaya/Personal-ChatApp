#include <QApplication>
#include "LoginWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setStyle("Fusion");
    
    LoginWindow loginWindow;
    loginWindow.show();
    
    return app.exec();
}