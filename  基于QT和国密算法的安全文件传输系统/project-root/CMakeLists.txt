cmake_minimum_required(VERSION 3.10)
project(SimpleGUI)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 COMPONENTS Widgets Network Sql REQUIRED)  # 添加Sql模块
# 设置 GmSSL 路径
set(GMSSL_ROOT "$ENV{HOME}/GmSSL-3.1.1-Linux")
include_directories(${GMSSL_ROOT}/include)
link_directories(${GMSSL_ROOT}/lib)

# 添加 dbmanager 库
add_library(dbmanager STATIC 
    src/database/dbmanager.cpp
    src/database/dbmanager.h
)
# 设置目标属性
target_include_directories(dbmanager PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
)
target_link_libraries(dbmanager
    Qt6::Core
    Qt6::Sql
    gmssl
)


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
    gmssl
)

# ================== Server ==================
add_executable(server
    src/server/main.cpp
    src/server/auditlogger.cpp # <-- 添加审计功能的源文件
    src/server/auditlogger.h   # <-- 添加审计功能的头文件
)
target_link_libraries(server PRIVATE
        Qt6::Network
        Qt6::Core  # 确保包含 Core 模块
        dbmanager  # 添加数据库管理器库
)


# ================== 目录设置 ==================
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/src/database
)



