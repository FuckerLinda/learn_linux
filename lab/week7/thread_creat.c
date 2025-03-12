/* thread_creat.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct student {
    int age;
    char name[20];
} STU;

void *create(void *arg) {
    STU *temp = (STU *)arg;
    printf("The following is transferred to thread\n");
    printf("STU age is %d\n", temp->age);
    printf("STU name is %s\n", temp->name);
    return NULL; // 补充返回值
}

int main() {
    pthread_t tidp;
    STU *stu = malloc(sizeof(STU));
    stu->age = 20;
    strcpy(stu->name, "abcdefg");
    int error = pthread_create(&tidp, NULL, create, (void *)stu);
    if (error != 0) {
        printf("pthread_create failed\n");
        return -1;
    }
    pthread_join(tidp, NULL);
    free(stu); // 释放内存
    return 0;
}
