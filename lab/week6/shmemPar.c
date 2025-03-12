// shmemPar.c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SIZE 1024

int main() {
    int shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | 0600);
    char *shmaddr;
    pid_t pid = fork();

    if (pid == 0) { // 子进程写入数据
        shmaddr = shmat(shmid, NULL, 0);
        strcpy(shmaddr, "Hello from child!");
        shmdt(shmaddr);
    } else { // 父进程读取数据
        sleep(1); // 等待子进程写入
        shmaddr = shmat(shmid, NULL, 0);
        printf("Parent received: %s\n", shmaddr);
        shmdt(shmaddr);
        shmctl(shmid, IPC_RMID, NULL); // 删除共享内存
    }
    return 0;
}
