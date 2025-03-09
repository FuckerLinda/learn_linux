#!/bin/bash

# 创建项目目录结构
mkdir -p project/{src,lib,build}

# 生成源码文件
cat > project/src/main.c << 'EOF'
#include <stdio.h>
#include "../lib/func.h"  // 包含库头文件

int main() {
    double x = 4.0;
    printf("sqrt(%f) = %f\n", x, my_sqrt(x));  // 使用库函数
    return 0;
}
EOF

cat > project/lib/func.c << 'EOF'
#include <math.h>

double my_sqrt(double x) {
    return sqrt(x);  // 实际调用数学库
}
EOF

cat > project/lib/func.h << 'EOF'
#ifndef FUNC_H
#define FUNC_H

double my_sqrt(double x);

#endif
EOF

# 生成有问题的顶层Makefile（未声明依赖）
cat > project/Makefile << 'EOF'
all:
	$(MAKE) -C lib
	$(MAKE) -C src

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C src clean
	rm -rf build/*
EOF

# 生成lib目录Makefile
cat > project/lib/Makefile << 'EOF'
LIB = libfunc.a
OBJ = func.o

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

func.o: func.c
	gcc -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)
EOF

# 生成src目录Makefile（包含-L路径）
cat > project/src/Makefile << 'EOF'
all: ../build/main

../build/main: main.o
	gcc -o $@ $< -L../lib -lfunc

main.o: main.c
	gcc -I../lib -c $< -o $@

clean:
	rm -f main.o ../build/main
EOF

echo "项目结构创建完成"
echo "重现错误步骤："
echo "1. cd project"
echo "2. make -j       # 应该会报错找不到libfunc.a"
echo
echo "修复步骤："
echo "修改顶层Makefile，替换为显式依赖的版本后再次运行 make -j"
