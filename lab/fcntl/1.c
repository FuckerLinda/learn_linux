#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main() {
    int fd = open("test_fifo", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // 设置非阻塞模式
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    char buf[10];
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n == -1 && errno == EAGAIN) {
        printf("Non-blocking read: No data available.\n");
    } else {
        printf("Read %zd bytes.\n", n);
    }

    close(fd);
    return 0;
}
