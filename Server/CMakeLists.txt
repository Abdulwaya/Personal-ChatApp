project(ChatServer)

set(CMAKE_AUTOMOC ON)

add_executable(ChatServer
    main.cpp
    ChatServer.h
    ChatServer.cpp
    ClientHandler.h
    ClientHandler.cpp
    DatabaseManager.h
    DatabaseManager.cpp
)

target_link_libraries(ChatServer
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    ChatShared
)