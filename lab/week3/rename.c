#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define BUFFERSIZE 512

void debug(char *mess, char *param, int n) {
    if (n == -1) {
        printf("Error occurred: %s %s\n", mess, param);
        exit(1);
    }
}

int main(int ac, char **av) {
    int in_fd, out_fd, n_chars;
    char buf[BUFFERSIZE];
    if (ac != 3) {
        printf("Usage: %s source destination\n", av[0]);
        exit(1);
    }
    in_fd = open(av[1], O_RDONLY);
    debug("Cannot open", av[1], in_fd);
    out_fd = creat(av[2], 0744);
    debug("Cannot create", av[2], out_fd);
    while ((n_chars = read(in_fd, buf, BUFFERSIZE)) > 0) {
        if (write(out_fd, buf, n_chars) != n_chars) {
            debug("Write error to", av[2], -1);
        }
    }
    close(in_fd);
    close(out_fd);
    return 0;
}
