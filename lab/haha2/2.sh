#!/bin/bash
# ...（原创建目录和文件的命令）
cd project-root
exec $SHELL  # 替换当前 Shell 进程以保持路径
