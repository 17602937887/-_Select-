/*************************************************************************
> File Name: select.c
> Author: 
> Mail: 
> Created Time: 2019年08月02日 星期五 15时13分27秒
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
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>

#define IP "0.0.0.0"
#define PORT 6666

char name[BUFSIZ][BUFSIZ];

struct Node{
    int num;
    char name[BUFSIZ];
}Node[BUFSIZ];

int main()
{
    char buf[BUFSIZ], ip_addr[BUFSIZ];
    int lfd = socket(AF_INET, SOCK_STREAM, 0), cfd;
    struct sockaddr_in server_addr, client_addr;
    int i, opt = 1, n, max_index, max_fd, ready, client[FD_SETSIZE];
    fd_set now_fdset, all_fdset;
    socklen_t client_len;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr.s_addr);

    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(lfd,  128);

    memset(client, -1, sizeof(client));
    max_fd = lfd;
    max_index = -1;

    FD_ZERO(&all_fdset);
    FD_SET(lfd, &all_fdset);

    while(1)
    {
        now_fdset = all_fdset;
        ready = select(max_fd + 1, &now_fdset, NULL, NULL, NULL);
        if(FD_ISSET(lfd, &now_fdset))
        {
            client_len = sizeof(client_addr);
            cfd = accept(lfd, (struct sockaddr *)&client_addr, &client_len);
            FD_SET(cfd, &all_fdset);
            printf("IP = %s, PORT = %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_addr, sizeof(ip_addr)), htons(client_addr.sin_port));
            for(i = 0; i < FD_SETSIZE; i++)
            {
                if(client[i] < 0)
                {
                    client[i] = cfd;
                    break;
                }
            }
            if(i > max_index)
                max_index = i;
            if(cfd > max_fd)
                max_fd = cfd;
            memset(buf, 0, sizeof(buf));
            read(cfd, buf, sizeof(buf));
            memset(name[cfd], 0, sizeof(name[cfd]));
            strcpy(name[cfd], buf);
            int ans = 0;
            for(int i = 0; i <= max_index; i++)
            {
                if(client[i] > 0 && client[i] != cfd)
                {
                    char tmp_s[BUFSIZ];
                    strcpy(tmp_s, name[cfd]);
                    strcat(tmp_s, "上线了，欢迎!\n");
                    write(client[i], tmp_s, strlen(tmp_s));
                }
            }
            if(--ready == 0)
                continue;
        }
        for(int i = 0; i <= max_index; i++)
        {
            if(client[i] < 0)
                continue;
            if(FD_ISSET(client[i], &now_fdset))
            {
                memset(buf, 0, sizeof(buf));
                n = read(client[i], buf, sizeof(buf));
                if(strlen(buf) == 0 && n > 0)
                    continue;
                if(strcmp(buf, "exit\n") == 0 || n <= 0)
                {
                    printf("client[%d] = %d exit\n", i, client[i]);
                    for(int j = 0; j <= max_index; j++)
                    {
                        if(client[j] > 0 && j != i)
                        {
                            char tmp_s[BUFSIZ];
                            strcpy(tmp_s, name[client[i]]);
                            strcat(tmp_s, "下线了，大家拜拜\n");
                            write(client[j], tmp_s, strlen(tmp_s));
                        }
                    }
                    FD_CLR(client[i], &all_fdset);
                    client[i] = -1;
                    --ready;
                    continue;
                }
                for(int j = 0; j <= max_index; j++)
                {
                    if(client[j] > 0 && i != j)
                    {
                        char tmp_s[BUFSIZ];
                        strcpy(tmp_s, name[client[i]]);
                        strcat(tmp_s, "对大家说:");
                        strcat(tmp_s, buf);
                        write(client[j], tmp_s, strlen(tmp_s));
                    }
                }
                ready--;
            }
            if(ready == 0)
                break;
        }
    }
    close(lfd);
    return 0;
}
