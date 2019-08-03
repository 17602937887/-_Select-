/*************************************************************************
> File Name: client.c
> Author: 
> Mail: 
> Created Time: 2019年07月27日 星期六 17时57分56秒
************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#define IP "39.106.120.32"
#define PORT 6666

int main()
{
    char buf[BUFSIZ];
    int cfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_len;

    cfd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr.s_addr);

    connect(cfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("输入您的昵称用来进行聊天\n");
    scanf("%s", buf);
    write(cfd, buf, strlen(buf));
    pid_t pid = fork();
    if(pid == 0)
    {
        while(1)
        {
            close(STDIN_FILENO);
            memset(buf, 0, sizeof(buf));
            read(cfd, buf, sizeof(buf));
            printf("%s", buf);
        }
    }
    else if(pid > 0)
    {
        while(1)
        {
            memset(buf, 0, sizeof(buf));
            read(STDIN_FILENO, buf, sizeof(buf));
            write(cfd, buf, sizeof(buf));
        }
    }
    close(cfd);
    return 0;
}
