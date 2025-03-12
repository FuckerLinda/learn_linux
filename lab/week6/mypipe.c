/* mypipe.c */
#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<sys/types.h>
#include <stdlib.h>

int main() {
    int pid, mypipe[2];
    pipe(mypipe);
    pid = fork();

    if (pid < 0) {
        perror("create process failed\n");
        exit(0);
    }

    if (pid == 0) { // 子进程执行 grep init
        close(mypipe[1]);
        dup2(mypipe[0], 0); // 将管道读端重定向到标准输入
        close(mypipe[0]);
        execlp("grep", "grep", "init", NULL);
    } else { // 父进程执行 ps aux
        close(mypipe[0]);
        dup2(mypipe[1], 1); // 将管道写端重定向到标准输出
        close(mypipe[1]);
        execlp("ps", "ps", "aux", NULL);
    }
}
