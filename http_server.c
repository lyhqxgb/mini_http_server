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

void request_200(int client_socket, const char *path);

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
        if(client_socket < 0) print_err("accept");

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
            if(res == 0){
                request_200(client_socket, url_path);
            }else{
                //读取文件信息错误，返回404
                request_404(client_socket);
            }
        }else{
            request_501(client_socket);
        }


        // close(client_socket);
        //close方法不会等输出结束，因此需要用shutdown先关闭输出，再用close关闭socket
        shutdown(client_socket, SHUT_WR);
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
    const char *res = "HTTP/1.1 404 Not Found\r\n\
Connection: keep-alive\r\n\
Content-Type: text/html; charset=utf-8\r\n\
Server: Leo\r\n\
\r\n\
<HTML>\
<HEAD>\
<TITLE>File Not Found</TITLE>\
<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\
</HEAD>\
<BODY>\
    <P>Server can not find file.</p>\
</BODY>\
</HTML>";

    write(client_socket, res, strlen(res));
}

void request_200(int client_socket, const char *path)
{
    const char *main_header = "HTTP/1.0 200 OK\r\nServer: Martin Server\r\nContent-Type: text/html\r\nConnection: Close\r\n";

    char content_len[50] = {0};
    char send_buf[64]; //要读的buff
    int buff_size = sizeof(send_buf);
    int size = 0; //文件大小

    //写请求头
    int len = write(client_socket, main_header, strlen(main_header));

    //计算文件大小
    FILE *file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    
    sprintf(content_len, "Content-Length: %d\r\n\r\n", size);
    write(client_socket, content_len, strlen(content_len));

    //输出响应内容
    rewind(file);
    while(!feof(file)){
        fread(send_buf, buff_size, 1, file);
        write(client_socket, send_buf, buff_size);
        if(ferror(file)) print_err("read file error");
    }

    fclose(file);

}