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

#define MAXLINE 1024
#define LISTENQ 5

void error(const char *fmt, ...);

int main() {
    int listenfd, connfd;
    struct sockaddr_in server_address;
    char buff[MAXLINE];
    time_t ticks;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(13);
    bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(listenfd, LISTENQ);
    for (; ; ) {
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        close(connfd);
    }
    return 0;
}

void error(const char *fmt, ...) {
    va_list ptr;
    va_start(ptr, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, ptr);
    fprintf(stderr, "\n");
    va_end(ptr);
    exit(1);
}

