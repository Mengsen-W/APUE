/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-13 09:32:03
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 11:39:10
 * @Description : epoll server
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

static int SERV_PORT = 6666;
static int OPEN_MAX = 5000;

int main(int argc, char *argv[]) {
  char buf[BUFSIZ];
  char str[INET_ADDRSTRLEN];
  struct epoll_event tep, ep[OPEN_MAX];
  int efd;
  int nready;
  socklen_t clilen;
  int sockfd;

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("socket()");
    exit(1);
  }

  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in servaddr, clieaddr;
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

  efd = epoll_create(OPEN_MAX);
  if (efd == -1) {
    perror("epoll_create()");
    exit(1);
  }

  tep.events = EPOLLIN;
  tep.data.fd = listenfd;

  int res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
  if (res == -1) {
    perror("epoll_ctl()");
    exit(1);
  }

  while (1) {
    nready = epoll_wait(efd, ep, OPEN_MAX, -1);
    if (nready == -1) {
      perror("epoll_wait()");
      exit(1);
    }
    for (int i = 0; i < nready; ++i) {  // 遍历触发事件套接字
      if (!(ep[i].events & EPOLLIN)) continue;
      if (ep[i].data.fd == listenfd) {
        clilen = sizeof(clieaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&clieaddr, &clilen);
        printf("received from %s at PORT %d \n",
               inet_ntop(AF_INET, &clieaddr.sin_addr, str, sizeof(str)),
               ntohs(clieaddr.sin_port));

        tep.events = EPOLLIN;
        tep.data.fd = connfd;
        res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
        if (res == -1) {
          perror("epoll_ctl()");
          exit(1);
        }
      } else {  // 实际业务套接字
        sockfd = ep[i].data.fd;
        int n = read(sockfd, buf, BUFSIZ);
        if (n == 0) {  // 对方关闭
          res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, nullptr);
          if (res == -1) {
            perror("epoll_ctl()");
            exit(-1);
          }
          close(sockfd);
          printf("closed %d\n", sockfd);
        } else if (n < 0) {  // 出错
          perror("read()");
          continue;
        } else {                                                 // 业务
          for (int j = 0; j < n; ++j) buf[j] = toupper(buf[j]);  // 实际业务
          int we = write(STDOUT_FILENO, buf, n);
          if (we == -1) perror("write");
          we = write(sockfd, buf, n);
          if (we == -1) perror("write");
        }
      }
    }
  }
  close(listenfd);
  return 0;
}