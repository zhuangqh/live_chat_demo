#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024
#define SERVER_PORT 8000
#define max(x, y) x > y ? x : y

void error(const char *fmt, ...) {
    va_list ptr;
    va_start(ptr, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ptr);
    fprintf(stderr, "\n");
    va_end(ptr);
    exit(1);
}

void chat_cli(FILE *fp, int sockfd) {
    int maxfdp1, stdineof, n;
    fd_set rset;
    char buf[MAXLINE], name[MAXLINE];
    stdineof = 0;
    FD_ZERO(&rset);
    fputs("WHO R U: ", stdout);
    fgets(name, MAXLINE, stdin);
    for (;;) {
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, buf, MAXLINE)) == 0) {
                if (stdineof == 1)
                    return;
                else
                    error("chat_cli: server terminated prematurely");
            }
            write(fileno(stdout), strcat(name, buf), n);
        }
        if (FD_ISSET(fileno(fp), &rset)) {
            if ((n = read(fileno(fp), buf, MAXLINE)) == 0) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            write(sockfd, buf, n);
        }
    }
}

