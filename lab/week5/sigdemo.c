#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

void sigHandler(int signalNum) {
    printf("The sign no is:%d\n", signalNum);
}

int main() {
    signal(SIGINT, sigHandler);        // 处理Ctrl+C信号
    signal(SIGQUIT, SIG_IGN);          // 忽略Ctrl+\信号
    while(True) {                         // 改为while(1)避免依赖stdbool.h
        sleep(1);
    }
    return 0;
}
