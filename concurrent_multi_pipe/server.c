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
int pipe_fd[MAX_CONNECTIONS][2];
static void boardcast(long, int);

int main() {
  int listenfd, i, j;
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
      client[i] = pipe_fd[i][0] = pipe_fd[i][1] = -1;
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

        // create pipe
        if (pipe(pipe_fd[i]) == 0) {

          if ((pid = fork()) == -1)
            error("fork failed\n");
          if (pid == 0) {
            printf("child process begin\n");
            close(listenfd);
            boardcast(connfd, pipe_fd[i][0]);
            exit(0);
          }

          // update data
          for (j = 0; j < MAX_CONNECTIONS; j++) {
            if (pipe_fd[j][0] != -1) {
              write(pipe_fd[j][1], client, sizeof(int) * MAX_CONNECTIONS);
              printf("update in %d\n", j);
            }
          }
          printf("close child process's connfd in father process\n");
          close(connfd);
        } else {
          error("create pipe error");
        }
    }
    return 0;
}

static void boardcast(long connfd, int fd) {
  int true_client[MAX_CONNECTIONS];
  ssize_t n;
  char buf[MAXLINE];
  int j, tmpfd;

  while ((n = read(connfd, buf, MAXLINE)) > 0) {
    // read data from pipe
    read(fd, true_client, sizeof(int) * MAX_CONNECTIONS);
    printf("connfd %ld ", connfd);
    for (int i = 0; i < 3; ++i) {
      printf("%d ", true_client[i]);
    }
    printf("\n");
    close(fd);
    printf("enter boardcast\n");
    for (j = 0; j < MAX_CONNECTIONS; j++) {
      if ((tmpfd = true_client[j]) < 0 || connfd == tmpfd)
        continue;
      printf("cast\n");
      write(tmpfd, buf, n);
    }
  }
  printf("end of boardcast\n");
  close(connfd);
}

