#include <QCoreApplication>
#include "ChatServer.h"
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "Starting Chat Server...";
    
    ChatServer server;
    if (!server.startServer(12345)) {
        qDebug() << "Failed to start server!";
        return 1;
    }
    
    qDebug() << "Server started on port 12345";
    qDebug() << "Waiting for connections...";
    
    return app.exec();
}