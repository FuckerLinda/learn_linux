/* multthread.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *t1_execute(void *arg) {
    while (1) {
        printf("in thread1\n");
        sleep(1);
    }
    return NULL;
}

void *t2_execute(void *arg) {
    sleep(2);
    pthread_exit(NULL); // 替换为 exit(0) 对比效果
}

int main() {
    pthread_t pid1, pid2;
    pthread_create(&pid1, NULL, t1_execute, NULL);
    pthread_create(&pid2, NULL, t2_execute, NULL);
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    return 0;
}
