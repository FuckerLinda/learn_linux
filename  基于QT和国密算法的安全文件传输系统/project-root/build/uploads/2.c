#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

volatile sig_atomic_t child_ready = 0;
volatile sig_atomic_t child_exited = 0;
volatile sig_atomic_t signal_count = 0;

// 子进程准备就绪信号处理
void handle_sigusr1(int sig) {
    child_ready = 1;
}

// SIGCHLD处理函数
void handle_sigchld(int sig) {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        child_exited = 1;
    }
}

// 子进程的SIGUSR2处理函数
void handle_sigusr2(int sig) {
    signal_count++;
    if (signal_count >= 5) {
        const char *msg = "Received 5 signals, child exit!\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        exit(EXIT_SUCCESS);
    }
}

int main() {
    pid_t pid;
    struct sigaction sa;

    // 设置SIGCHLD处理
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);

    // 创建子进程
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // 子进程
        // 设置SIGUSR2处理
        struct sigaction sa_child;
        memset(&sa_child, 0, sizeof(sa_child));
        sa_child.sa_handler = handle_sigusr2;
        sigaction(SIGUSR2, &sa_child, NULL);

        // 通知父进程已准备好
        kill(getppid(), SIGUSR1);

        // 等待信号
        while (1) pause();

    } else { // 父进程
        // 设置SIGUSR1处理（等待子进程准备就绪）
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handle_sigusr1;
        sigaction(SIGUSR1, &sa, NULL);


        // 每2秒发送SIGUSR2
        while (!child_exited) {
            kill(pid, SIGUSR2);
            sleep(2);
        }

        printf("Child terminated.\n");
    }
    return EXIT_SUCCESS;
}
