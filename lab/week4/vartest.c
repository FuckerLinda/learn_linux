#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
int glob = 10; // 全局变量
int main(void) {
    int local;
    pid_t pid;
    local = 8;
    if ((pid = fork()) == 0) {
        // 子进程
        sleep(2);
    } else if (pid > 0) {
        // 父进程
        glob++;
        local--;
        sleep(10);
    }
    printf("glob = %d, local = %d, mypid = %d\n", glob, local, getpid());
    return 0;
}
