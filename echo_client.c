#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define SERVER_PORT 666
#define SERVER_IP "127.0.0.1"

void show_error(char *msg)
{
    printf("error: %s %s\n", msg, strerror(errno));
    exit(errno);
}

int main(int argc, char *argv[])
{
    int sock;
    char *message;
    struct sockaddr_in server_addr;
    int n, tmp;
    char buf[64];

    if(argc != 2){
        fputs("Usage: ./echo_client message \n", stderr);
        exit(1);
    }

    message = argv[1];
    printf("message: %s\n", message);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) show_error("socket");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    tmp = connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(tmp == -1) show_error("connect");

    write(sock, message, strlen(message));
    n = read(sock, buf, sizeof(buf) - 1);

    if(n > 0){
        buf[n] = '\0';
        printf("receive message: %s\n", buf);
    }else{
        fputs("read error!\n", stderr);
        exit(1);
    }

    printf("finished!\n");
    close(sock);

    return 0;
}