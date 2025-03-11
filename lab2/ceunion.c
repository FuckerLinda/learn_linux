#include <stdio.h>

void check_endian_union() {
    union {
        unsigned int i;
        unsigned char c[4];
    } test_union;

    test_union.i = 0x12345678;

    if (test_union.c[0] == 0x78) {
        printf("Little Endian (Union Method)\n");
    } else {
        printf("Big Endian (Union Method)\n");
    }
}

int main() {
    check_endian_union();
    return 0;
}
