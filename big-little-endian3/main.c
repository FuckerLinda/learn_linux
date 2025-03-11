#include <stdio.h>
#include <string.h>
#include "ble.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [method]\n", argv[0]);
        printf("Methods: bitfield, bitwise, standard, pointer, union\n");
        return 1;
    }

    if (strcmp(argv[1], "bitfield") == 0) {
        check_endian_bit_field();
    } else if (strcmp(argv[1], "bitwise") == 0) {
        check_endian_bitwise();
    } else if (strcmp(argv[1], "standard") == 0) {
        check_endian_standard();
    } else if (strcmp(argv[1], "pointer") == 0) {
        check_endian_pointer();
    } else if (strcmp(argv[1], "union") == 0) {
        check_endian_union();
    } else {
        printf("Invalid method. Available methods: bitfield, bitwise, standard, pointer, union\n");
        return 1;
    }

    return 0;
}
