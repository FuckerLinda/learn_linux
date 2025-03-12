/* pipedemo.c */
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>

char sendbuf[] = "I am Linux";
char recbuf[20];
char parrecbuf[20];

void reverse(char *str1) {
    if (str1 == NULL) return;
    char *p = str1;
    char *q = str1;
    while (*q) q++;
    q--;
    while (q > p) {
        char t = *p;
        *p++ = *q;
        *q-- = t;
    }
}

int main() {
    int mypipe[2], fd;
    if (pipe(mypipe) < 0) { perror("pipe failed"); exit(0); }
    if ((fd = fork()) < 0) { perror("fork failed"); exit(0); }

    if (fd == 0) { // 子进程
        read(mypipe[0], recbuf, strlen(sendbuf));
        printf("The child process get %s\n", recbuf);
        reverse(recbuf);
        write(mypipe[1], recbuf, strlen(recbuf));
    }

    if (fd > 0) { // 父进程
        write(mypipe[1], sendbuf, strlen(sendbuf));
        sleep(1); // 等待子进程处理（原代码为sleep(10)，缩短等待时间）
        read(mypipe[0], parrecbuf, strlen(sendbuf));
        printf("The parent process get %s\n", parrecbuf);
        wait(NULL); // 防止僵尸进程
    }
    return 0;
}
