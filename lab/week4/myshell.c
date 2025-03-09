#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
void split(char **arr, char *str, const char *del) {
    char *s = strtok(str, del);
    while (s != NULL) {
        *arr++ = s;
        s = strtok(NULL, del);
    }
}
int main() {
    char *args[10];
    char arg[100];
    pid_t pid;
    int status;
    while (1) {
        printf("please input command:\n");
        memset(args, 0, sizeof(args));
        if (fgets(arg, sizeof(arg), stdin) == NULL) break; // 替换不安全的gets
        arg[strcspn(arg, "\n")] = '\0'; // 去除换行符
        if (strcmp("#", arg) == 0) break;
        split(args, arg, " ");
        pid = fork();
        if (pid < 0) {
            printf("fork failed\n");
            exit(0);
        } else if (pid == 0) {
            execvp(args[0], args); // 修正参数传递
            perror("exec error");
            exit(1);
        } else {
            wait(&status);
        }
    }
    return 0;
}
