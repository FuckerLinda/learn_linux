#!/bin/bash

# 创建项目目录结构
mkdir -p project/src project/lib

# 生成顶层Makefile（错误的构建顺序：先src后lib）
cat > project/Makefile << 'EOF'
all:
	$(MAKE) -C src
	$(MAKE) -C lib
EOF

# 生成主程序源码
cat > project/src/main.c << 'EOF'
#include "func.h"
#include <stdio.h>

int main() {
    printf("Hello from main!\n");
    func(); // 依赖libfunc.a中的函数
    return 0;
}
EOF

# 生成src目录的Makefile
cat > project/src/Makefile << 'EOF'
CC = gcc
CFLAGS = -I../lib
LDFLAGS = -L../lib -lfunc

all: main

main: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f main main.o
EOF

# 生成库函数实现
cat > project/lib/func.c << 'EOF'
#include "func.h"
#include <stdio.h>

void func() {
    printf("Library function called\n");
}
EOF

# 生成库头文件
cat > project/lib/func.h << 'EOF'
#ifndef FUNC_H
#define FUNC_H

void func(void);

#endif
EOF

# 生成lib目录的Makefile
cat > project/lib/Makefile << 'EOF'
CC = gcc
AR = ar
CFLAGS = -c

all: libfunc.a

libfunc.a: func.o
	$(AR) rcs $@ $<

func.o: func.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.a
EOF
