#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    // 打开一个文件（假设文件名为 "test.txt"）
    int fd = open("test.txt", O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 设置非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        close(fd);
        return 1;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL (O_NONBLOCK)");
        close(fd);
        return 1;
    }
    printf("Non-blocking mode set.\n");

    // 取消追加模式
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        close(fd);
        return 1;
    }
    flags &= ~O_APPEND;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl F_SETFL (~O_APPEND)");
        close(fd);
        return 1;
    }
    printf("Append mode disabled.\n");

    // 关闭文件
    close(fd);
    return 0;
}
