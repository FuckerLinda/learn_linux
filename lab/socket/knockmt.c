#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 30000
#define MAX_QUEUE 10

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void *client_handler(void *arg) {
    int connect_d = *(int *)arg;
    free(arg); // 释放动态分配的内存
    
    pthread_t tid = pthread_self();
    printf("Thread ID: %lu\n", (unsigned long)tid);
    
    const char *steps[] = {
        "Internet Knock-Knock Protocol Server\r\nVersion 1.0\r\nKnock! Knock!\r\n> ",
        "Oscar\r\n> ",
        "Oscar silly question, you get a silly answer.\r\n"
    };
    char buffer[256];
    
    send(connect_d, steps[0], strlen(steps[0]), 0);
    recv(connect_d, buffer, sizeof(buffer), 0);
    send(connect_d, steps[1], strlen(steps[1]), 0);
    recv(connect_d, buffer, sizeof(buffer), 0);
    send(connect_d, steps[2], strlen(steps[2]), 0);
    
    close(connect_d);
    return NULL;
}

int main() {
    int listener_d = socket(PF_INET, SOCK_STREAM, 0);
    if (listener_d == -1)
        error("Can't open socket");
    
    struct sockaddr_in name;
    name.sin_family = PF_INET;
    name.sin_port = htons(PORT);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(listener_d, (struct sockaddr *)&name, sizeof(name)) == -1)
        error("Can't bind to socket");
    
    if (listen(listener_d, MAX_QUEUE) == -1)
        error("Can't listen");
    
    printf("Multi-threaded server running on port %d...\n", PORT);
    
    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t address_size = sizeof(client_addr);
        int *connect_d = malloc(sizeof(int));
        *connect_d = accept(listener_d, (struct sockaddr *)&client_addr, &address_size);
        if (*connect_d == -1)
            error("Can't accept connection");
        
        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, connect_d);
        pthread_detach(tid); // 分离线程，避免资源泄漏
    }
    
    close(listener_d);
    return 0;
}
