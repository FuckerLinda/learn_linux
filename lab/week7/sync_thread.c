
/* sync_thread.c */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define MAX 100
static sem_t sem1, sem2;
static int global = 1;

void *t1_exe(void *arg) {
    while (global < MAX) {
        sem_wait(&sem1);
        printf("Thread1: global=%d -> %d\n", global, global + 1);
        global++;
        sem_post(&sem2);
        sleep(1);
    }
    return NULL;
}

void *t2_exe(void *arg) {
    while (global < MAX) {
        sem_wait(&sem2);
        printf("Thread2: global=%d -> %d\n", global, global * 2);
        global *= 2;
        sem_post(&sem1);
        sleep(1);
    }
    return NULL;
}

int main() {
    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 0);
    pthread_t pid1, pid2;
    pthread_create(&pid1, NULL, t1_exe, NULL);
    pthread_create(&pid2, NULL, t2_exe, NULL);
    pthread_join(pid1, NULL);
    pthread_join(pid2, NULL);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    return 0;
}
