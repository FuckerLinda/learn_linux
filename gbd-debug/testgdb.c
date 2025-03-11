#include <stdio.h>

int sum(int m);

int main()
{
    int *pi;
    int i, n = 0;
    pi = &i;
    int arr[3] = { 10, 20, 30 };
    pi = arr;
    sum(50);
    for (i = 0; i <= 50; i++) {
	n += i;
    }

    printf("The sum of 1-50 is %d \n", n);
}

int sum(int m)
{
    int i, n = 0;
    for (i = 1; i <= m; i++) {
	n += i;
    }

    printf("The sum of 1-50 is %d \n", n);

    return n;
}
