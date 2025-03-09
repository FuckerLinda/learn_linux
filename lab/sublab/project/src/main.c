#include "func.h"
#include <stdio.h>

int main() {
    printf("Hello from main!\n");
    func(); // 依赖libfunc.a中的函数
    return 0;
}
