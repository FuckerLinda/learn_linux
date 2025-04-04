cmake_minimum_required(VERSION 3.10)
project(SimpleGUI)

# 查找QT库
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# 添加可执行文件
add_executable(simple_gui
    src/main.cpp
    gui/mainwindow.cpp
    gui/ui/mainwindow.ui
)

# 链接QT库
target_link_libraries(simple_gui PRIVATE Qt6::Widgets)


# 添加头文件搜索路径
include_directories(
    ${CMAKE_SOURCE_DIR}
)
