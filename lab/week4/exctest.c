#include <unistd.h>
#include <stdio.h>
#include <string.h>
int main() {
    char *arglist[3];
    pid_t pid;
    arglist[0] = "ls";
    arglist[1] = "-l"; // 修正参数为 "-l"
    arglist[2] = NULL; // 修正结束符为NULL
    pid = fork();
    if (pid == 0) {
        execvp("ls", arglist);
        printf("*** program is over. bye\n"); // 若exec失败才会执行
    }
    return 0;
}
