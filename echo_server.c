#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define SOCKET_PORT 666 

void show_error(char *msg)
{
    printf("error: %s %s\n", msg, strerror(errno));
    exit(errno);
}

int main(void)
{
    int sock;
    int tmp;
    //创建socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) show_error("socket");

    //创建ip地址
    struct sockaddr_in sock_addr;
    //清空变量
    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY); //设置地址
    sock_addr.sin_port = htons(SOCKET_PORT); //设置端口

    //绑定地址到socket
    tmp = bind(sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if(tmp == -1) show_error("bind");

    //监听socket
    tmp = listen(sock, 128);
    if(tmp == -1) show_error("listen");

    while(1){
        //获取客户端ip
        struct sockaddr_in client_addr;
        int client_sock, len, i;
        socklen_t client_addr_len;
        char client_ip[32] = {0};
        char buf[256] = {0};

        client_sock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
        if(client_sock == -1) show_error("accept");

        //打印客户端ip和端口号
        printf("client ip: %s\t port: %d\n",
            inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
            ntohs(client_addr.sin_port)
        );

        //读取客户端信息
        len = read(client_sock, buf, sizeof(buf) - 1);
        buf[len] = '\0';
        printf("receive[%d]: %s\n", len, buf);

        //将客户端信息转为大写
        for(i = 0; i<len; i++){
            if(buf[i] >= 'a' && buf[i] <= 'z'){
                buf[i] = buf[i] - ('a' - 'A');
            }
        }
        

        //返回信息给客户端
        len = write(client_sock, buf, len);
        printf("finished. len:%d\n", len);
        close(client_sock);
    }

    close(sock);

    return 0;
}
