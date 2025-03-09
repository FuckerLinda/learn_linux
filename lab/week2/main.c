#include <stdio.h>
#include "calc.h"
int main(int argc, char* argv[]) {
    double v1, v2, m, sum2;
    v1 = 3.2;
    v2 = 8.9;
    m = aver(v1, v2);
    sum2 = sum(v1, v2);
    printf("The mean of %3.2f and %3.2f is %3.2f\n", v1, v2, m);
    printf("The sum of %3.2f and %3.2f is %3.2f\n", v1, v2, sum2);
    return 0;
}
