#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include "rocerror.h"  // 新增头文件

int main(int argc, char *argv[]) {
    char filename[PATH_MAX] = {0};

    if (argc != 2) {
        app_error("You must supply a filename as an argument");  // 修改为 app_error
        return 1;
    }

    strncpy(filename, argv[1], sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';

    if (creat(filename, 0644) == -1) {
        unix_error("Can't create file");  // 修改为 unix_error
        return 1;
    }

    return 0;
}
