#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

int i = 0;

void alarmsign_handler(int SignNo) {
    printf("%d seconds\n", ++i);
}

int main() {
    signal(SIGALRM, alarmsign_handler);
    struct itimerval tval;

    // 初始间隔1秒，后续间隔1秒
    tval.it_value.tv_sec = 1;
    tval.it_value.tv_usec = 0;
    tval.it_interval.tv_sec = 1;
    tval.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &tval, NULL);
    while(getchar() != '#');          // 输入#结束程序
    return 0;
}
