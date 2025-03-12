/* withmutex.c */
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char str[] = "abcdefghijklmnopqrstuvwxyz123456789";
pthread_mutex_t mutex;
int index2 = 0;

void *t1_exe(void *arg) {
    while (index2 < strlen(str) - 1) {
        pthread_mutex_lock(&mutex);
        printf("The %dth element is %c\n", index2, str[index2]);
        index2++;
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t pid1, pid2;
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&pid1, NULL, t1_exe, NULL);
    pthread_create(&pid2, NULL, t1_exe, NULL); // 使用相同函数
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}
