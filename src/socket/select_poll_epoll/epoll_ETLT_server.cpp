/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-13 11:38:54
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 13:07:16
 * @Description: ET / LT server 非阻塞IO
 */

//! nonblock ET 最牛逼

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

static int SERV_PORT = 6666;
static int OPEN_MAX = 5000;
static int MAXLINE = 12;

int main(int argc, char *argv[]) {
  char buf[BUFSIZ];
  char str[INET_ADDRSTRLEN];
  struct epoll_event tep, ep[OPEN_MAX];
  int efd;
  socklen_t clilen;

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

  // tep.events = EPOLLIN;
  tep.events = EPOLLIN | EPOLLET;
  // 只调用 wait 1次

  int res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
  if (res == -1) {
    perror("epoll_ctl()");
    exit(1);
  }

  clilen = sizeof(clieaddr);
  int connfd = accept(listenfd, (struct sockaddr *)&clieaddr, &clilen);
  printf("received from %s at PORT %d \n",
         inet_ntop(AF_INET, &clieaddr.sin_addr, str, sizeof(str)),
         ntohs(clieaddr.sin_port));

  int flag = fcntl(connfd, F_GETFL);
  flag |= O_NONBLOCK;
  fcntl(connfd, F_SETFL, flag);

  tep.data.fd = connfd;
  epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);

  while (1) {
    res = epoll_wait(efd, ep, 10, -1);
    printf("res = %d\n", res);
    if (ep[0].data.fd == connfd) {
      int len = 0;  // 通过非阻塞IO用边缘触发达到水平触发
      while (1) {
        len = read(connfd, buf, MAXLINE / 2);
        printf("write()begin = len  = %d\n", len);
        sleep(2);
        if (len > 0) {
          int we = write(STDOUT_FILENO, buf, len);
          if (we < 0) {
            perror("write()");
            exit(1);
          }
        } else if (len == 0) {
          printf("write() end\n");
          close(connfd);
          break;
        } else {
          perror("write()\n");
          // EAGAIN 这个在非阻塞上面是正常的
          printf("%d\n",(errno));
        }
      }
    }
  }

  close(listenfd);
  close(connfd);
  return 0;
}