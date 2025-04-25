#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int set_nonblock(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = inet_addr(SERVER_IP)
    };

    set_nonblock(sockfd);  // 设置为非阻塞模式

    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    char buffer[BUFFER_SIZE];
    while(1) {
        ssize_t n = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
        if(n > 0) {
            buffer[n] = '\0';
            printf("Received: %s\n", buffer);
            break;
        } else if(n < 0) {
            if(errno == EWOULDBLOCK) {
                printf("Waiting for data...\n");
                sleep(1);
                continue;
            }
            perror("recv error");
            break;
        } else {
            printf("Connection closed\n");
            break;
        }
    }
    close(sockfd);
    return 0;
}
