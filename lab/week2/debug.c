#include <stdio.h>
void main()
{
    int arr[10], i = 0;
    for (i = 0; i < 10; i++)
    {
        arr[i] = i;
        if (DEBUG) // 使用了一个名为 DEBUG 的宏，该宏在编译的时候定义
        {
            printf("arr[%d]=%d\n", i, arr[i]);
        }
    }
}
