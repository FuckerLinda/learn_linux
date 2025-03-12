#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int sock;
    int addr_len;
    int len;
    char buff[128];
    struct sockaddr_in s_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket create failed");
        exit(1);
    }

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(8888);
    memset(s_addr.sin_zero, '\0', sizeof(s_addr.sin_zero));

    if (argc > 2) {
        s_addr.sin_addr.s_addr = inet_addr(argv[1]);
        strcpy(buff, argv[2]);
    } else {
        printf("input server ip and parameter!\n");
        exit(0);
    }

    addr_len = sizeof(s_addr);
    len = sendto(sock, buff, strlen(buff), 0, (struct sockaddr *)&s_addr, addr_len);
    if (len < 0) {
        perror("send error.\n");
        exit(1);
    }

    sleep(1);
    len = recvfrom(sock, buff, sizeof(buff)-1, 0, (struct sockaddr *)&s_addr, &addr_len);
    if (len < 0) {
        perror("recvfrom error.\n");
        exit(1);
    }
    printf("receive from server: %s\n", buff);
    close(sock);
    return 0;
}
