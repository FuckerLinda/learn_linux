#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>

void sigchild_handler(int sig) {
    int status;
    waitpid(-1, &status, WNOHANG);
    if (WIFEXITED(status))
        printf("child process exit normally\n");
    else if (WIFSIGNALED(status))
        printf("child process exit abnormally\n");
    else if (WIFSTOPPED(status))
        printf("child process is stopped\n");
    else
        printf("unknown status\n");
}

int main() {
    pid_t pid;
    signal(SIGCHLD, sigchild_handler);
    pid = fork();
    if (pid == 0) {
        abort();                     // 子进程异常退出
    } else if (pid > 0) {
        sleep(2);
        printf("parent process\n");
    } else {
        exit(1);
    }
    return 0;
}
