#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int fd[2];
    pid_t pid;
    char buf[1024];

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // 父进程
        close(fd[0]); // 关闭读端
        char *str = "20242922卞聪";
        write(fd[1], str, strlen(str) + 1); // 写入含'\0'
        close(fd[1]); // 关闭写端
        wait(NULL); // 等待子进程
    } else { // 子进程
        close(fd[1]); // 关闭写端
        ssize_t n = read(fd[0], buf, sizeof(buf));
        printf("Received: %s\n", buf);
        close(fd[0]);
        exit(EXIT_SUCCESS);
    }
    return 0;
}
