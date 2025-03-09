#include <stdio.h>
#include <unistd.h>
void parent_code(int delay) {
    sleep(delay);
}
int main() {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        exit(0); // 子进程立即退出
    } else if (pid > 0) {
        parent_code(100000); // 父进程长期睡眠
    }
    return 0;
}
