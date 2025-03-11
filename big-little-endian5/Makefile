# 编译器配置
CC = gcc
CFLAGS = -I./include -Wall -Wextra -pedantic

# 目录定义
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 源文件和目标文件列表
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/main

# 默认目标：编译可执行文件
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# 编译每个源文件为目标文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译产物
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/main

.PHONY: all clean
