// semexample.c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>

union semun { int val; };

int main() {
    int sem_id = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    union semun arg;
    arg.val = 0;
    semctl(sem_id, 0, SETVAL, arg); // 初始化信号量为0

    if (fork() == 0) { // 子进程
        printf("Child running\n");
        sleep(1);
        struct sembuf op = {0, 1, 0}; // V操作
        semop(sem_id, &op, 1);
    } else { // 父进程
        struct sembuf op = {0, -1, 0}; // P操作
        semop(sem_id, &op, 1);
        printf("Parent running\n");
        semctl(sem_id, 0, IPC_RMID); // 删除信号量
    }
    return 0;
}
