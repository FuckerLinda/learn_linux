#include <stdio.h>

void check_endian_pointer() {
    unsigned int i = 0x12345678;
    char *c = (char*)&i;

    if (*c == 0x78) {
        printf("Little Endian (Pointer Method)\n");
    } else {
        printf("Big Endian (Pointer Method)\n");
    }
}

int main() {
    check_endian_pointer();
    return 0;
}
