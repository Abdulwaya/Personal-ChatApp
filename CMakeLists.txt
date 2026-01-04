cmake_minimum_required(VERSION 3.16)
project(ChatApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

add_subdirectory(Shared)
add_subdirectory(Server)
add_subdirectory(Client)