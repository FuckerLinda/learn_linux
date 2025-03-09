#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    int fd = open("a.txt", O_RDWR);
    off_t oldpos = lseek(fd, strlen("abcdefghijklm1"), SEEK_SET);
    if (oldpos != -1) {
        int result = write(fd, "uuuuu", strlen("uuuuu"));
        if (result == -1) {
            printf("write error\n");
        }
    } else {
        printf("lseek error");
    }
    close(fd);
    return 0;
}
