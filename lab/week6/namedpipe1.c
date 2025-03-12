#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    // 添加此头文件以支持 O_RDONLY
#include <unistd.h>   // 添加此头文件以支持 read()
#include <error.h>

#define N 20

int main() {
    int fd = open("/tmp/mypipe", O_RDONLY); // 确保命名管道已创建
    char buf[N];
    if (fd != -1) printf("FIFO file is opened\n");
    if (read(fd, buf, N) != -1)
        printf("Received: %s\n", buf);
    else
        perror("read error");
    close(fd);
    exit(EXIT_SUCCESS);
}
