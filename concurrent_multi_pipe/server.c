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


int client[FD_SETSIZE];
int pipe_fd[FD_SETSIZE][2];
static void to_boardcast(long, int);
static void *Listen(void *arg);
pid_t pid;
int maxfd, maxi;
fd_set allset, rset;
int start;

int main() {
  int i, nready, sockfd, j, n, tmpfd;
  char buf[MAXLINE];
  pthread_t tid;
  // thread to listen message send from client
  pthread_create(&tid, NULL, &Listen, NULL);

  for (i = 0; i < FD_SETSIZE; ++i)
    pipe_fd[i][0] = pipe_fd[i][1] = -1;
  FD_ZERO(&allset);
  maxfd = -1;
  maxi = -1;
  start = 0;
  for (;;) {
    if (start) {
      rset = allset;
      nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
      for (i = 0; i <= maxi; ++i) {
        if ((sockfd = pipe_fd[i][0]) < 0)
          continue;

        if (FD_ISSET(sockfd, &rset)) {

          if ((n = read(sockfd, buf, MAXLINE)) == 0) {
            printf("before clean\n");
            close(sockfd);
            FD_CLR(sockfd, &allset);
            client[i] = -1;
          } else {
            buf[n] = '\0';
            printf("recv %s\n", buf);

            for (j = 0; j <= maxi; ++j) {
              if ((tmpfd = client[j]) < 0 || j == i)
                continue;
              write(tmpfd, buf, n);
            }
          }
        }
      }
    }
  }
  return 0;
}

static void *Listen(void *arg) {
  pthread_detach(pthread_self());
  int listenfd, i;
  long connfd;
  socklen_t clilen;
  printf("hello\n");
  struct sockaddr_in client_address, server_address;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&server_address, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(SERVER_PORT);
  bind(listenfd, (struct sockaddr *)&server_address, sizeof(server_address));
  listen(listenfd, LISTENQ);
  for (i = 0; i < FD_SETSIZE; i++)
    client[i] = pipe_fd[i][0] = pipe_fd[i][1] = -1;

  for (;;) {
    clilen = sizeof(client_address);
    connfd = accept(listenfd, (struct sockaddr *)&client_address, &clilen);

    for (i = 0; i < FD_SETSIZE; i++)
      if (client[i] < 0) {
        client[i] = connfd;
        break;
      }
    if (i == FD_SETSIZE)
      error("too many clients");
    if (i > maxi) {
      maxi = i;
    }

    // create pipe
    if (pipe(pipe_fd[i]) != 0) {
      error("create pipe error");
    } else {

      if ((pid = fork()) == -1)
        error("fork failure\n");
      if (pid == 0) {
        close(listenfd);
        to_boardcast(connfd, pipe_fd[i][1]);
        exit(0);
      }

      FD_SET(pipe_fd[i][0], &allset);
      if (pipe_fd[i][0] > maxfd)
        maxfd = pipe_fd[i][0];
      if (!start) start = 1;
    }
  }
  return NULL;
}

/**
 * receive message send from client
 * pipe it to the main process
 * boardcast in main process
 */
static void to_boardcast(long connfd, int pipefd) {
  ssize_t n;
  char buf[MAXLINE];

  while ((n = read(connfd, buf, MAXLINE)) > 0) {
    buf[n] = '\0';
    printf("enter boardcast with str %s length %lu\n", buf, strlen(buf));
    write(pipefd, buf, strlen(buf));
  }
  close(connfd);
}
