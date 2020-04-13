/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-12 18:03:56
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 09:59:34
 * @Description: poll server
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stropts.h>
#include <unistd.h>

static int SERV_PORT = 6666;
static int OPEN_MAX = 1024;

int main(int argc, char *argv[]) {
  int i = 0, j = 0, maxi = 0, listenfd = 0, connfd = 0, sockfd = 0;
  int nready = 0;
  ssize_t n = 0;
  char buf[BUFSIZ], str[INET_ADDRSTRLEN];
  socklen_t len = 0;
  struct pollfd client[OPEN_MAX];
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("socket()");
    exit(1);
  }

  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  int be = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if (be == -1) {
    perror("bind()");
    exit(1);
  }

  int le = listen(listenfd, SOMAXCONN);
  if (le == -1) {
    perror("listen()");
    exit(1);
  }
  client[0].fd = listenfd;
  client[0].events = POLLIN;
  printf("Hello poll\n");

  // 用-1初始化剩余文件描述符，0也是文件描述符
  for (i = 1; i < OPEN_MAX; ++i) client[i].fd = -1;

  while (true) {
    nready = poll(client, maxi + 1, -1);
    if (nready < 0) {
      perror("poll()");
      exit(1);
    }

    printf("poll()\n");

    if (client[0].revents & POLLIN) {
      len = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
      printf("received from %s at PORT %d \n",
             inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
             ntohs(cliaddr.sin_port));

      for (i = 1; i < OPEN_MAX; ++i) {
        if (client[i].fd < 0) {
          client[i].fd = connfd;
          client[i].events = POLLIN;
          break;
        }
      }
      if (i == OPEN_MAX) {
        fputs("too many client connections\n", stderr);
        exit(1);
      }
      if (i > maxi) maxi = i;
      if (--nready <= 0) continue;
    }
    for (i = 0; i <= maxi; ++i) {
      if ((sockfd = client[i].fd) < 0) continue;
      if (client[i].revents & POLLIN) {
        if ((n = read(sockfd, buf, BUFSIZ)) < 0) {
          printf("close()");
          close(sockfd);
          client[i].fd = -1;
        } else if (n == 0) {
          printf("close()");
          close(sockfd);
          client[i].fd = -1;
        } else {
          for (j = 0; j < n; ++j) buf[j] = toupper(buf[j]);  // 实际业务
          printf("toupper\n");
          int we = write(STDOUT_FILENO, buf, n);
          if (we == -1) perror("write");
          we = write(sockfd, buf, n);
          if (we == -1) perror("write");
        }
        if (--nready == 0) break;
      }
    }
  }
  close(listenfd);
  return 0;
}