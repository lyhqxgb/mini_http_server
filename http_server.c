#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 80

void print_err(char *msg)
{
    fprintf(stderr, "%s[%d]: %s", msg, errno, strerror(errno));
    exit(errno);
}

int main(void)
{
    int server_socket = 0;
    struct sockaddr_in sock_addr;
    int res;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        print_err("socket");
    }

    int on = 1;
    res = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    if(res < 0) print_err("setsockopt");

    bzero(&sock_addr, sizeof(sock_addr));

    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(SERVER_PORT);

    res = bind(server_socket, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if(res != 0) print_err("bind");

    res = listen(server_socket, 128);
    if(res != 0) print_err("listen");

    while(1){
        int client_socket = 0;
        struct sockaddr_in client_addr;
        socklen_t addr_len;
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if(res != 0) print_err("accept");
    }

    close(server_socket);

    return 0;
}