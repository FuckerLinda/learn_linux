#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main() {
    int fd = open("lockfile", O_CREAT | O_RDWR, 0644);
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0  // Lock entire file
    };

    // 父进程加写锁
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("fcntl lock");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程尝试加锁（非阻塞）
        if (fcntl(fd, F_SETLK, &lock) == -1) {
            printf("Child: Lock is held by parent.\n");
        }
        _exit(0);
    } else {
        wait(NULL);
    }

    close(fd);
    return 0;
}
