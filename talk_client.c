#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_PORT 777
#define SERVER_ADDR "127.0.0.1"

void show_error(char *msg)
{
    printf("error: %s %s\n", msg, strerror(errno));
    exit(errno);
}

int main(void)
{
    int sock, res;
    struct sockaddr_in sock_addr;
    char buf[256] = {0};
    int len = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) show_error("socket");

    memset(&sock_addr, 0, sizeof(sock_addr));
    inet_pton(AF_INET, SERVER_ADDR, &sock_addr.sin_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(SERVER_PORT);

    res = connect(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if(res == -1) show_error("connect");

    printf("连接成功，请输入要发送的内容\n");
    fgets(buf, sizeof(buf), stdin);

    write(sock, buf, strlen(buf));

    while( res = read(sock, buf, sizeof(buf)-1) ){
        printf("receive[%d]: %s\n", res, buf);
        printf("请输入要发送的内容\n");
        fgets(buf, sizeof(buf), stdin);
        write(sock, buf, strlen(buf));
    }

    printf("server closed.\n");
    close(sock);

    return 0;
}