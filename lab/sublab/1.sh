#!/bin/bash

# 创建项目目录结构
mkdir -p project/{src,lib}

# 生成顶层 Makefile
cat > project/Makefile << 'EOF'
all: lib_build src_build

lib_build:
	$(MAKE) -C lib

src_build: lib_build
	$(MAKE) -C src

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C src clean
EOF

# 生成 src/main.c
cat > project/src/main.c << 'EOF'
#include <stdio.h>
#include "../lib/func.h"

int main() {
    double x = 4.0;
    printf("sqrt(%f) = %f\n", x, my_sqrt(x));
    return 0;
}
EOF

# 生成 src/Makefile
cat > project/src/Makefile << 'EOF'
build/main: main.o
	mkdir -p ../build
	gcc -o ../build/main main.o -L../lib -lfunc -lm

main.o: main.c
	gcc -c main.c -o main.o -I../lib

clean:
	rm -f main.o ../build/main
EOF

# 生成 lib/func.h
cat > project/lib/func.h << 'EOF'
#ifndef FUNC_H
#define FUNC_H

double my_sqrt(double x);

#endif
EOF

# 生成 lib/func.c
cat > project/lib/func.c << 'EOF'
#include "func.h"
#include <math.h>

double my_sqrt(double x) {
    return sqrt(x);
}
EOF

# 生成 lib/Makefile
cat > project/lib/Makefile << 'EOF'
all: libfunc.a

libfunc.a: func.o
	ar rcs $@ $^

func.o: func.c
	gcc -c -o func.o func.c -lm

clean:
	rm -f *.o *.a
EOF

echo "项目结构已创建完成！"
