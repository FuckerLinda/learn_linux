#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void alarm_handler(int sig) {
    printf("sleep is over\n");
}

void mysleep(int seconds) {
    signal(SIGALRM, alarm_handler);
    alarm(seconds);
    pause();
}

int main() {
    printf("before pause\n");
    mysleep(3);
    printf("after pause\n");
    return 0;
}
