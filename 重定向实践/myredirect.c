#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // 检查参数合法性
    if (argc < 5 || strcmp(argv[1], "-t") != 0) {
        fprintf(stderr, "Usage: %s -t n command outputfile\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int type = atoi(argv[2]);
    if (type < 1 || type > 3) {
        fprintf(stderr, "Error: n must be 1, 2, or 3\n");
        exit(EXIT_FAILURE);
    }

    char *command = argv[3];
    char *outputfile = argv[4];

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // 子进程处理重定向
        int fd;

        switch (type) {
            case 1: // 方法1：关闭标准输出后打开文件
                close(STDOUT_FILENO);
                fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open failed (method 1)");
                    exit(EXIT_FAILURE);
                }
                break;

            case 2: // 方法2：使用 dup 复制文件描述符
                fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open failed (method 2)");
                    exit(EXIT_FAILURE);
                }
                close(STDOUT_FILENO);
                if (dup(fd) == -1) {
                    perror("dup failed");
                    exit(EXIT_FAILURE);
                }
                close(fd);
                break;

            case 3: // 方法3：使用 dup2 直接重定向
                fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open failed (method 3)");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2 failed");
                    exit(EXIT_FAILURE);
                }
                close(fd);
                break;
        }

        // 执行命令
        execlp(command, command, (char *)NULL);
        perror("execlp failed"); // 若 exec 失败才执行到这里
        exit(EXIT_FAILURE);
    } else { // 父进程等待子进程
        wait(NULL);
    }

    return 0;
}
