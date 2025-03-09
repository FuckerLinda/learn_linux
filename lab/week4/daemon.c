#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#define MAXFILE 65535
int main() {
    pid_t pc;
    int i, fd, len;
    char *buf = "Hello,everybody!\n";
    len = strlen(buf);
    pc = fork();
    if (pc < 0) {
        printf("fork error\n");
        exit(1);
    } else if (pc > 0) {
        exit(0); // 父进程退出
    }
    setsid();    // 修正为setsid()
    chdir("/");
    umask(0);
    for (i = 0; i < MAXFILE; i++) close(i); // 关闭所有文件描述符
    while (1) {
        fd = open("/tmp/daemon.log", O_CREAT | O_WRONLY | O_APPEND, 0600);
        if (fd < 0) {
            perror("open");
            exit(1);
        }
        write(fd, buf, len);
        close(fd);
        sleep(10);
    }
    return 0;
}
