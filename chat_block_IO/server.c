#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "awesome_handler.h"

#define LISTENQ 5

int main() {
    int i, j, maxi, maxfd, listenfd, connfd, sockfd, tmpfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in client_address, server_address;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);
    bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(listenfd, LISTENQ);
    /* for (; ; ) { */
            /* connfd = accept(listenfd, (struct sockaddr *)NULL, NULL); */
            /* ticks = time(NULL); */
            /* snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); */
            /* write(connfd, buff, strlen(buff)); */
            /* close(connfd); */
        /* } */
    maxfd = listenfd;
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(client_address);
            connfd = accept(listenfd, (struct sockaddr *)&client_address, &clilen);
            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            if (i == FD_SETSIZE)
                error("too many clients");
            FD_SET(connfd, &allset);
            if (connfd > maxfd)
                maxfd = connfd;
            if (i > maxi)
                maxi = i;
            if (--nready <= 0)
                continue;
        }
        for (i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ((n = read(sockfd, buf, MAXLINE)) == 0) {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    for (j = 0; j <= maxi; j++) {
                        if ((tmpfd = client[j]) < 0)
                            continue;
                        write(tmpfd, buf, n);
                    }
                }
                if (--nready <= 0)
                    break;
            }
        }
    }
    return 0;
}

