#include <stdio.h>
#include "../lib/func.h"  // 包含库头文件

int main() {
    double x = 4.0;
    printf("sqrt(%f) = %f\n", x, my_sqrt(x));  // 使用库函数
    return 0;
}
