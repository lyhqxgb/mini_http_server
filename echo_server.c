#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SOCKET_PORT 666 

int main(void)
{
    int sock;
    //创建socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    //创建ip地址
    struct sockaddr_in sock_addr;
    //清空变量
    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); //设置地址
    sock_addr.sin_port = htons(SOCKET_PORT); //设置端口

    //绑定地址到socket
    bind(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr));

    //监听socket
    listen(sock, 128);

    return 0;
}
