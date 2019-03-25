#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define  TALK_PORT 777

void show_error(char *msg)
{
    printf("error: %s %s\n", msg, strerror(errno));
    exit(errno);
}

int readLine(int socket, char *buf, int size);

int main(void)
{
    int server_socket, res;
    struct sockaddr_in sock_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) show_error("socket");

    int on = 1;
    res = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    if(res < 0) show_error("setsockopt");

    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(TALK_PORT);

    res = bind(server_socket, (struct sockaddr *) &sock_addr, sizeof(sock_addr));
    if(res == -1) show_error("bind");

    res = listen(server_socket, 128);
    if(res == -1) show_error("listen");

    while(1){
        int client_sock, len, res;
        struct sockaddr_in client_addr;
        socklen_t addrlen;
        char buf[256];
        char talk1[] = "Do you like C++?";
        char answer1[] = "Yes, I do.";
        char talk2[] = "Why do you like C++?";
        char answer2[] = "Because I can use it to write programs.\nAnd you?";
        int status = 1; //表示对话仍在继续

        addrlen = sizeof(client_addr);
        client_sock = accept(server_socket, (struct sockaddr *) &client_addr, &addrlen);
        if(client_sock == -1) show_error("accept");

        while(status){
            len = readLine(client_sock, buf, sizeof(buf) - 1);
            if(len < 1){
                //读取结束
                status = 0;
                break;
            }
            buf[len] = '\0';
            printf("receive[%d]: %s\n", len, buf);

            if(0 == strcmp(buf, talk1)){
                write(client_sock, answer1, sizeof(answer1));
            } else if(0 == strcmp(buf, talk2)) {
                write(client_sock, answer2, sizeof(answer2));
                printf("start to end\n");
                status = 0;

            }else{
                write(client_sock, buf, len);
                status = 0;
            }
        }
        
        printf("close socket\n");
        close(client_sock);

    }

    close(socket);

}

int readLine(int socket, char *buf, int size)
{
    int i;
    char c;
    int len; //read方法读取到的长度

    for(i = 0; i < size; i++)
    {
        c = 0;
        len = read(socket, &c, 1);
        if(len == 0){
            //说明会话结束
            return len;
        }
        if(c && (c == '\n' || c == '\r')){
            break;
        }
        buf[i] = c;
    }
    buf[i] = '\0';

    return i;
    
}