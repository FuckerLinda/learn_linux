#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
int main() {
    char buf[10];
    char *str1 = "This is child process";
    char *str2 = "This is parent process";
    pid_t pid;
    int fd, readsize;
    fd = open("test.txt", O_WRONLY); // 修正为O_WRONLY
    if (fd == -1) {
        perror("open failed");
        exit(0);
    }
    readsize = read(fd, buf, 5); // 读取5字节
    pid = fork();
    switch (pid) {
        case -1:
            perror("fork failed");
            exit(0);
        case 0:
            write(fd, str1, strlen(str1));
            break;
        default:
            write(fd, str2, strlen(str2));
    }
    close(fd);
    return 0;
}
