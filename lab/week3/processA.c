// processA.c
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void debug(char *mess, char *param, int n) {
    if (n == -1) {
        printf("Error: %s %s\n", mess, param);
        exit(1);
    }
}

int main() {
    int fd = open("A.log", O_WRONLY | O_APPEND); // 关键：使用 O_APPEND
    debug("Cannot open", "A.log", fd);
    sleep(10);
    int nwrite = write(fd, "AAAAAAAAAAAAAA", strlen("AAAAAAAAAAAAAA"));
    debug("Write error", "A.log", nwrite);
    close(fd);
    return 0;
}

// processB.c（与 processA.c 类似，写入内容为 "BBBBBBBBBB"）
