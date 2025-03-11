#include <stdio.h>

void check_endian_bit_field() {
    struct {
        unsigned int a:8;
        unsigned int b:8;
        unsigned int c:8;
        unsigned int d:8;
    } test;

    test.d = 0x12;
    test.c = 0x34;
    test.b = 0x56;
    test.a = 0x78;

    unsigned char *p = (unsigned char *)&test;


    if (*p == 0x78) {
        printf("Little Endian (Bit-field Method)\n");
    } else  {
        printf("Big Endian (Bit-field Method)\n");
    }
}

