// Copyright  rocedu@BESTI   rocedu@qq.com   All rights reserved.

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/limits.h>

int main(int argc, char *argv[]) {
    // 定义一个字符数组用于存储文件名，最大长度为PATH_MAX
    char filename[PATH_MAX] = { 0 };

    // 检查命令行参数的数量，应该为2（包含程序名和文件名）
    if (argc != 2) {
        fprintf(stderr, "You must supply a filename as an argument \n");
        return 1;  // 返回1表示错误
    }

    // 将命令行参数中的文件名拷贝到filename数组中，确保不超过PATH_MAX-1
    strncpy(filename, argv[1], PATH_MAX - 1);

    // 创建文件，权限设置为可读可写（0644）
    if (creat(filename, 0644) == -1) {
        fprintf(stderr, "Can't create file %s \n", filename);
        return 1;  // 返回1表示错误
    }

    return 0;  // 返回0表示程序执行成功
}

