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
#include <pthread.h>
#include "awesome_handler.h"

#define LISTENQ 5
#define MAX_CONNECTIONS 1024

int client[MAX_CONNECTIONS];
static void boardcast(long);

int main() {
    int listenfd, i;
    long connfd;
    pid_t pid;
    socklen_t clilen;
    struct sockaddr_in client_address, server_address;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);
    bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(listenfd, LISTENQ);
    for (i = 0; i < MAX_CONNECTIONS; i++)
        client[i] = -1;
    for (;;) {
        clilen = sizeof(client_address);
        connfd = accept(listenfd, (struct sockaddr *)&client_address, &clilen);
        for (i = 0; i < MAX_CONNECTIONS; i++)
            if (client[i] < 0) {
                client[i] = connfd;
                break;
            }
        if (i == MAX_CONNECTIONS)
            error("too many clients");
        printf("connfd %ld\n", connfd);

        if ((pid = fork()) == -1)
          error("fork failed\n");
        if (pid == 0) {
          printf("child process begin\n");
          close(listenfd);
          boardcast(connfd);
          exit(0);
        }
        printf("close child process's connfd in father process\n");
        close(connfd);
    }
    return 0;
}

static void boardcast(long connfd) {
    ssize_t n;
    char buf[MAXLINE];
    int j, tmpfd;

    while ((n = read(connfd, buf, MAXLINE)) > 0) {
        for (j = 0; j < MAX_CONNECTIONS; j++) {
            if ((tmpfd = client[j]) < 0 || connfd == tmpfd)
                continue;
            printf("cast\n");
            write(tmpfd, buf, n);
        }
    }
    printf("end of boardcast\n");
    close(connfd);
}

