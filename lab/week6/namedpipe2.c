#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    // 添加此头文件以支持 O_WRONLY
#include <unistd.h>   // 添加此头文件以支持 write()
#include <error.h>

#define N 20

int main() {
    mkfifo("/tmp/mypipe", 0777); // 创建命名管道
    int fd = open("/tmp/mypipe", O_WRONLY);
    char buf[N];
    printf("Input string: ");
    scanf("%s", buf);
    if (write(fd, buf, sizeof(buf)) != -1)
        printf("Write success\n");
    close(fd);
    exit(EXIT_SUCCESS);
}
