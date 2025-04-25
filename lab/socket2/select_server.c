#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    fd_set readfds;
    int client_sockets[MAX_CLIENTS] = {0};
    
    printf("Server started on port %d\n", PORT);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_fd = server_fd;

        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
                if(client_sockets[i] > max_fd) 
                    max_fd = client_sockets[i];
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(server_fd, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
            
            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    printf("New connection: fd%d\n", new_socket);
                    break;
                }
            }
        }

        for(int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if(sd > 0 && FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t valread = read(sd, buffer, BUFFER_SIZE);
                
                if(valread == 0) {
                    close(sd);
                    client_sockets[i] = 0;
                    printf("Client fd%d disconnected\n", sd);
                } else {
                    printf("Received from fd%d: %s\n", sd, buffer);
                    send(sd, buffer, valread, 0);
                }
            }
        }
    }
    close(server_fd);
    return 0;
}
