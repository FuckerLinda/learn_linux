#include <signal.h>

void sigHandler(int signalNum) {
    printf("The sign no is:%d\n", signalNum);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND; // 关键：处理完信号后恢复默认行为

    sigaction(SIGINT, &sa, NULL);
    signal(SIGQUIT, SIG_IGN);

    while(1) {
        sleep(1);
    }
    return 0;
}
