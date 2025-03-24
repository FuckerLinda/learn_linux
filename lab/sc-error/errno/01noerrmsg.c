// Copyright  rocedu@BESTI   rocedu@qq.com   All rights reserved.

/*
 * 该程序用于创建一个文件，文件名通过命令行参数传入。
 * 如果未提供文件名，则程序会输出错误信息。
 */

#include <stdio.h>             // 包含标准输入输出库
#include <fcntl.h>            // 包含文件控制选项
#include <string.h>           // 包含字符串处理函数
#include <linux/limits.h>     // 包含系统限制常量，例如路径最大长度

int main(int argc, char *argv[]) {
    char filename[PATH_MAX] = { 0 }; // 存储文件名的缓冲区

    // 检查命令行参数的数量
    if (argc != 2) {
        // 输出错误信息到标准错误输出
        fprintf(stderr, "You must supply a filename as an argument\n");
        return 1; // 返回1表示出现错误
    }

    // 复制传入的文件名到filename变量，避免缓冲区溢出
    strncpy(filename, argv[1], PATH_MAX - 1);
    filename[PATH_MAX - 1] = '\0'; // 确保字符串以'\0'结尾

    // 创建文件，权限设置为644（可读写权限给文件所有者，读权限给其他用户）
    creat(filename, 00644);
    return 0; // 返回0表示成功结束
}

