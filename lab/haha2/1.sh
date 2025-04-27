mkdir -p project-root/{src,gui/ui,include,lib,docs} && \
touch project-root/src/main.cpp \
      project-root/gui/mainwindow.{h,cpp} \
      project-root/gui/ui/mainwindow.ui \
      project-root/{Makefile,CMakeLists.txt}

cd project-root

cat <<'EOF' > src/main.cpp
#include <QApplication>
#include "gui/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
EOF


cat <<'EOF' > gui/mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
};

#endif // MAINWINDOW_H
EOF



cat <<'EOF' > gui/mainwindow.cpp
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Simple GUI Demo");
    resize(400, 300);
}
EOF



cat <<'EOF' > gui/ui/mainwindow.ui
<?xml version="1.0" encoding="UTF-8"?>
<ui version="4.0">
 <class>MainWindow</class>
 <widget class="QMainWindow" name="MainWindow">
  <property name="windowTitle">
   <string>Simple GUI</string>
  </property>
  <property name="geometry">
   <rect>
    <x>0</x>
    <y>0</y>
    <width>400</width>
    <height>300</height>
   </rect>
  </property>
  <widget class="QWidget" name="centralWidget">
   <layout class="QVBoxLayout" name="verticalLayout">
    <item>
     <widget class="QLabel" name="label">
      <property name="text">
       <string>Hello World!</string>
      </property>
     </widget>
    </item>
   </layout>
  </widget>
 </widget>
</ui>
EOF






cat <<'EOF' > CMakeLists.txt
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
EOF


sudo apt install -y qt6-base-dev cmake build-essential
sudo apt install -y libgl1-mesa-dev mesa-common-dev libglu1-mesa-dev
sudo apt install -y qt6-base-dev qt6-qpa-plugins qt6-l10n-tools
mkdir build && cd build
cmake ..
make -j4
