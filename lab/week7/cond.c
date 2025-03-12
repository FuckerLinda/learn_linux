/* cond.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int i = 1;

void *thread1(void *arg) {
    for (i = 1; i <= 20; i++) {
        pthread_mutex_lock(&mutex);
        if (i % 9 == 0) {
            pthread_cond_signal(&cond);
        } else {
            printf("Thread1: %d\n", i);
        }
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

void *thread2(void *arg) {
    pthread_mutex_lock(&mutex);
    while (i < 20) {
        if (i % 9 == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Thread2: %d\n", i);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_t t_a, t_b;
    pthread_create(&t_a, NULL, thread2, NULL);
    pthread_create(&t_b, NULL, thread1, NULL);
    pthread_join(t_a, NULL);
    pthread_join(t_b, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
