#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>                   // 添加strncmp依赖的头文件

#define INPUTLEN 20
char input[INPUTLEN];

void inthandler(int s) {
    printf("I have Received signal %d ... waiting\n", s);
    sleep(2);
    printf("I am Leaving inthandler \n");
    signal(SIGINT, inthandler);       // 重新注册信号处理函数
}

void quithandler(int s) {
    printf("I have Received signal %d ... waiting\n", s);
    sleep(3);
    printf("I am Leaving quithandler \n");
    signal(SIGQUIT, quithandler);
}

int main() {                          // 修正main函数返回类型为int
    signal(SIGINT, inthandler);
    signal(SIGQUIT, quithandler);
    int nchars;
    do {
        printf("please input a message\n");
        nchars = read(0, input, INPUTLEN-1);
        if (nchars == -1) {
            perror("read returned an error");
        } else {
            input[nchars] = '\0';
            printf("You have inputed: %s\n", input);
        }
    } while (strncmp(input, "quit", 4) != 0);
    return 0;
}
