#include <stdio.h>
#include <endian.h>

void check_endian_standard() {
    if (__BYTE_ORDER == __LITTLE_ENDIAN) {
        printf("Little Endian (Standard Library Method)\n");
    } else if (__BYTE_ORDER == __BIG_ENDIAN) {
        printf("Big Endian (Standard Library Method)\n");
    }
}

