#include <sys/stat.h>
#include <stdio.h>
#include <string.h>  // 添加 string.h 头文件以使用 strcpy

void mode_to_letter(mode_t mode, char *str) {
    strcpy(str, "----------");

    // 文件类型
    if (S_ISDIR(mode)) str[0] = 'd';  // 目录
    if (S_ISCHR(mode)) str[0] = 'c';  // 字符设备
    if (S_ISBLK(mode)) str[0] = 'b';  // 块设备
    if (S_ISFIFO(mode)) str[0] = 'p'; // 管道
    if (S_ISLNK(mode)) str[0] = 'l';  // 符号链接
    if (S_ISSOCK(mode)) str[0] = 's'; // 套接字

    // 用户权限
    if (mode & S_IRUSR) str[1] = 'r'; // 用户读
    if (mode & S_IWUSR) str[2] = 'w'; // 用户写
    if (mode & S_IXUSR) str[3] = 'x'; // 用户执行

    // 组权限
    if (mode & S_IRGRP) str[4] = 'r'; // 组读
    if (mode & S_IWGRP) str[5] = 'w'; // 组写
    if (mode & S_IXGRP) str[6] = 'x'; // 组执行

    // 其他用户权限
    if (mode & S_IROTH) str[7] = 'r'; // 其他用户读
    if (mode & S_IWOTH) str[8] = 'w'; // 其他用户写
    if (mode & S_IXOTH) str[9] = 'x'; // 其他用户执行
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    struct stat attr;
    char mode[11];
    if (stat(argv[1], &attr) != -1) {
        mode_to_letter(attr.st_mode, mode);
        printf("File %s mode is: %s\n", argv[1], mode);
    } else {
        printf("Stat error\n");
    }
    return 0;
}
