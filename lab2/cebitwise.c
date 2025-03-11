#include <stdio.h>

void check_endian_bitwise() {
    unsigned int i = 0x12345678;

    if ((i >> 24) == 0x12) {
        printf("Little Endian (Bitwise Method)\n");
    } else {
        printf("Big Endian (Bitwise Method)\n");
    }
}

int main() {
    check_endian_bitwise();
    return 0;
}
