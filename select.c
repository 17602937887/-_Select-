/*************************************************************************
	> File Name: select.c
	> Author: 
	> Mail: 
	> Created Time: Mon 01 Apr 2019 09:55:55 AM CST
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>

#define IP "39.106.120.32"
#define PORT 6666

int main()
{
    int lfd, cfd, opt = 1, i, nowfd, n, j;
    int Ready, maxi, maxfd, client[FD_SETSIZE];
    fd_set allset, rset;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    char buf[BUFSIZ], str[INET_ADDRSTRLEN];
    lfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // 设置传输方式 IPV4 IPV6
    server_addr.sin_port = htons(PORT); // 设置服务器端口号 htons
    inet_pton(lfd, IP, &server_addr.sin_addr.s_addr); // 设置IP地址

    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 端口复用
    
    bind(lfd, (struct sockaddr *)&server_addr, sizeof(server_addr));// 绑定 ip 和 端口

    listen(lfd, 128); // 同时建立连接的上限

    maxfd = lfd;
    maxi = -1;
    memset(client, -1, sizeof(client));

    FD_ZERO(&allset); // 所有监听初始化
    FD_SET(lfd, &allset); // 将当前的建立连接的描述符加入监听集合

    while(1)
    {
        rset = allset; // 每次保存所有的集合
        Ready = select(maxfd + 1, &rset, NULL, NULL, NULL); // 返回监听满足的个数
        if(Ready < 0) // 发生错误;
        {
            perror("select error:");
        }
        if(FD_ISSET(lfd, &rset)) //判断当前有没有要建立连接的客户端
        {
            client_len = sizeof(client_addr);
            cfd = accept(lfd, (struct socket *)&client_addr, &client_len);
            FD_SET(cfd, &allset);
            printf("=======IP   is : %s ========",inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, str, sizeof(str)));
            printf("=======PORT is : %d ========\n", ntohs(client_addr.sin_port));
            for(i = 0; i < FD_SETSIZE; i++)
            {
                if(client[i] < 0)
                {
                    client[i] = cfd;
                    break;
                }
            }
            if(i == FD_SETSIZE)
            {
                printf("Too many client\n");
                exit(1);
            }
            if(cfd > maxfd)
            {
                maxfd = cfd;
            }
            if(i > maxi)
                maxi = i;
            if(--Ready == 0)
                continue;
        }
        for(i = 0; i <= maxi; i++)
        {
            if((nowfd = client[i]) < 0)
                continue;
            if(FD_ISSET(nowfd, &rset))
            {
                n = read(nowfd, buf, sizeof(buf));
                if(n == 0)
                {
                    close(nowfd);
                    FD_CLR(nowfd, &allset);
                    client[i] = -1;
                }
                else
                {
                    printf("data is : %s", buf);
                    for(j = 0; j <= maxi; j++) 
                    {
                        if(client[j] > 0 && client[j] != nowfd)
                        {
                            write(client[j], buf, n);
                        }
                    }
                }
                Ready--;
            }
            if(Ready == 0)
                break;
        }
    }
    close(lfd);

    return 0;
}
