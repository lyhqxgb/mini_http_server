#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#define SERVER_PORT 80
#define ROOT_PATH "/home/leo/share/mini_http_server"

void print_err(char *msg)
{
    fprintf(stderr, "%s[%d]: %s", msg, errno, strerror(errno));
    exit(errno);
}

int get_head_line(int socket, char *buff, int len);

int get_line_content(char *line, char *buff, unsigned int len, int sensitive);

void request_501(int client_socket);

void request_404(int client_socket);

int main(void)
{
    int server_socket = 0;
    struct sockaddr_in sock_addr;
    int res;
    char root[] = ROOT_PATH;

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
        socklen_t addr_len = sizeof(client_addr);
        char ip_addr[20] = {0};
        char line[200]; //记录一行请求信息
        char method[30]; //请求方法
        char url[200]; //请求的url
        int p_line = 0; //line读取到的当前位置
        char url_path[200] = {0}; //保存url的绝对路径
        struct stat file_info;

        memset(&client_addr, 0, sizeof(client_addr));
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if(res < 0) print_err("accept");

        //打印ip地址
        printf("ip: %s, port: %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_addr, sizeof(ip_addr)), ntohs(client_addr.sin_port));

        //打印http请求
        // while(res = get_head_line(client_socket, line, (int)sizeof(line))){
        //     printf("%s\n", line);
        // }

        //读取请求方法
        res = get_head_line(client_socket, line, (int)sizeof(line));
        if(!res) print_err("no head line");
        res = get_line_content(line, method, sizeof(method), 0);
        p_line += res;
        if(strcmp(method, "GET") == 0){
            //读取请求的url
            res = get_line_content(line + p_line, url, sizeof(url), 1);
            p_line += res;
            strcpy(url_path, root);
            strcat(url_path, url);
            printf("url_path: %s\n", url_path);

            //读取文件信息
            res = stat(url_path, &file_info);
            if(res != 0){
                //读取文件信息错误，返回404
                request_404(client_socket);
            }
        }else{
            request_501(client_socket);
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


/**
 * 获取一行中的内容
 * @param char *line
 * @param char *buff
 * @param unsigned int len
 * @param int sensitve
 * @return int
 */
int get_line_content(char *line, char *buff, unsigned int len, int sensitive)
{
    int i;
    char curr;
    int count = 0;

    //清空buff
    memset(buff, '\0', len);

    for(i = 0; i < len; i++){
        count++;
        curr = line[i];
        if(curr == '\r' || curr == '\n') continue;
        if(curr == ' ') break;
        if(!sensitive){
            buff[i] = toupper(curr);
        }else{
            buff[i] = curr;
        }
    }

    return count;
}

void request_501(int client_socket)
{
    int count;
    const char *res = "HTTP/1.0 501 Method Not Implemented\r\n\
Content-Type: text/html\r\n\
\r\n\
<HTML>\
<HEAD>\
<TITLE>Method Not Implemented</TITLE>\
</HEAD>\
<BODY>\
    <P>HTTP request method not supported.\
</BODY>\
</HTML>";

    count = write(client_socket, res, strlen(res));

    printf("count: %d\n", count);
    if(!count) print_err("write 501");
}

void request_404(int client_socket)
{
    int count;
    const char *res_head = "HTTP/1.0 404 Method NOT FOUND\r\nContent-Type: text/html\r\n\
\r\n\
<HTML>\n\
<HEAD>\n\
<TITLE>File Not Found</TITLE>\n\
</HEAD>\n\
<BODY>\n\
    <P>Server can not find file.</p>\n\
</BODY>\n\
</HTML>";

    count = write(client_socket, res_head, strlen(res_head));
    printf("%d \n%s", count, res_head);

    // //获取body的长度
    // sprintf(content_len, "Content-Length:%d\r\n\r\n", strlen(res_content));
    // printf("len[%d]: %s\n", strlen(res_content), content_len);
    // write(client_socket, content_len, strlen(content_len));

    // write(client_socket, res_head, strlen(res_head));
    // printf("%s\n", res_head);

    // if(!count) print_err("write 404");
}