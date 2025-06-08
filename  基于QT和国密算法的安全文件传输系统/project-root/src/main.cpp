#include <QApplication>
#include "gui/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window; // 创建主窗口
    window.show();     // 显示主窗口
    return app.exec(); // 启动事件循环
}
