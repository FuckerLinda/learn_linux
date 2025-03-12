#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>                   // 添加exit依赖的头文件

void intsig_handler(int signumber, siginfo_t* siginfo, void* empty) {
    printf("int handler, mypid=%d\n", getpid());
}

int main() {
    int pid;
    char* arg[] = {"ls", "-l", NULL}; // 修正execvp参数
    struct sigaction act;
    act.sa_sigaction = intsig_handler;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("install signal error");
        return 1;
    }

    printf("The parent pid=%d\n", getpid());
    pid = fork();
    if (pid < 0) {
        perror("fork failed!\n");
        exit(1);
    } else if (pid == 0) {
        execvp("ls", arg);           // 子进程执行ls命令
    } else {
        while(1);                    // 父进程无限循环
    }
    return 0;
}
