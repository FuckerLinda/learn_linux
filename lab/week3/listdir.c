#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

int main(int ac, char **av) {
    DIR *dir_ptr;
    struct dirent *dir;
    if (ac < 2) {
        printf("Usage: listdir directoryname\n");
        exit(1);
    }
    dir_ptr = opendir(av[1]);
    if (dir_ptr != NULL) {
        while ((dir = readdir(dir_ptr)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(dir_ptr);
    }
    return 0;
}
