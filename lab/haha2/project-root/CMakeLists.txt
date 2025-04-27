cmake_minimum_required(VERSION 3.10)
project(SimpleGUI)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

# Client (GUI)
add_executable(client
    src/main.cpp
    gui/mainwindow.cpp
    gui/ui/mainwindow.ui
)
target_link_libraries(client PRIVATE Qt6::Widgets Qt6::Network)

# Server
add_executable(server
    src/server/main.cpp
)
target_link_libraries(server PRIVATE Qt6::Network)

include_directories(${CMAKE_SOURCE_DIR})
