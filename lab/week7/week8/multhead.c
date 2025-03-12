/* multhead.c */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static int global = 1;

void *t1_execute(void *arg) {
    while (global < 100) {
        printf("The value is %4d\n", global);
        sleep(1); // 添加休眠避免忙等待
    }
    return NULL;
}

void *t2_execute(void *arg) {
    while (global < 100) {
        global++;
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t pid1, pid2;
    pthread_create(&pid1, NULL, t1_execute, NULL);
    pthread_create(&pid2, NULL, t2_execute, NULL);
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    return 0;
}
