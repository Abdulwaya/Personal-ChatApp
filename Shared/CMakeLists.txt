project(ChatShared)

add_library(ChatShared INTERFACE)
target_include_directories(ChatShared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ChatShared INTERFACE Qt6::Core)