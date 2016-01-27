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

void error(const char *fmt, ...);

int main(int argc, char **argv) {
    int sockfd, n;
    char receive_line[MAXLINE + 1];
    struct sockaddr_in server_address;
    if (argc != 2)
        error("usage: a.out <IP_address>");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("socket error");
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(13);
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0)
        error("inet_pton error for %s", argv[1]);
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        error("connect error");
    while ((n = read(sockfd, receive_line, MAXLINE)) > 0) {
        receive_line[n] = 0;
        if (fputs(receive_line, stdout) == EOF)
            error("fputs error");
    }
    if (n < 0)
        error("read error");
    exit(0);
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

