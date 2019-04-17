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

int get_head_line(int socket, char *buff, int len);

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
        char ip_addr[20] = {0};
        char line[200];

        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if(res != 0) print_err("accept");

        //打印ip地址
        printf("ip: %s, port: %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_addr, sizeof(ip_addr)), ntohs(client_addr.sin_port));

        //打印http请求
        while(res = get_head_line(client_socket, line, (int)sizeof(line))){
            printf("%s\n", line);
        }

        close(client_socket);
    }

    close(server_socket);

    return 0;
}


/**
 * 读取一行信息
 * @param buff 保存读取的内容
 * @param int buff的长度
 * @return 读取的字符数
 */
int get_head_line(int socket, char *buff, int len)
{
    int count = 0; //读取的字符数
    char curr; //当前读取到的字符
    char prev; //前一个字符
    int res = 0; //read的返回值
    int i = 0; //buff的当前位置

    memset(buff, '\0', sizeof(buff));
    
    for(i = 0; i<len; i++)
    {
        res = read(socket, &curr, 1);
        if(curr == '\r'){
            prev = curr;
            continue;
        }
        if(curr == '\n'){
            if(prev == '\r'){
                //说明到了行尾
                break;
            }
        }
        buff[i] = curr;
        count++;
    }
    
    return count;
}