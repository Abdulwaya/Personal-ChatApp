project(ChatClient)

set(CMAKE_AUTOMOC ON)

add_executable(ChatClient
    main.cpp
    LoginWindow.h
    LoginWindow.cpp
    MainWindow.h
    MainWindow.cpp
    ChatWidget.h
    ChatWidget.cpp
    NetworkManager.h
    NetworkManager.cpp
)

target_link_libraries(ChatClient
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    ChatShared
)

if(WIN32)
    set_target_properties(ChatClient PROPERTIES WIN32_EXECUTABLE TRUE)
endif()