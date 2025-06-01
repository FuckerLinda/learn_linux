cmake_minimum_required(VERSION 3.10)
project(SimpleGUI)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 COMPONENTS Widgets Network Sql REQUIRED)  # 添加Sql模块

# ================== Client (GUI) ==================
add_executable(client
    src/main.cpp
    # 新增文件
    gui/authwidgets.h	
    gui/authwidgets.cpp
    src/database/dbmanager.cpp
    src/database/dbmanager.h
    # 原有文件
    gui/mainwindow.cpp
    gui/mainwindow.h
)

target_link_libraries(client PRIVATE 
    Qt6::Widgets 
    Qt6::Network
    Qt6::Sql  # 新增SQL依赖
)

# ================== Server ==================
add_executable(server
    src/server/main.cpp
)
target_link_libraries(server PRIVATE
       	Qt6::Network
	Qt6::Core  # 确保包含 Core 模块
	)

# ================== 目录设置 ==================
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/src/database
)
