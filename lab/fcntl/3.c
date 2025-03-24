#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd1 = open("dup_test.txt", O_CREAT | O_WRONLY, 0644);
    int fd2 = fcntl(fd1, F_DUPFD, 0);

    write(fd1, "Hello ", 6);
    write(fd2, "World\n", 6);

    close(fd1);
    close(fd2);
    return 0;
}
