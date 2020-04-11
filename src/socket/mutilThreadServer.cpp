/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-11 17:39:56
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-11 18:01:33
 * @Description: 多线程版本
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERV_PORT 8888

struct s_info {
  struct sockaddr_in cliaddr;
  int connfd;
};

void *do_worker(void *arg) {
  struct s_info *ts = (struct s_info *)arg;
  char buf[BUFSIZ];

  printf("child ID %ld\n", pthread_self());
  while (1) {
    int len = read(ts->connfd, buf, sizeof(buf));
    if (len == 0) {
      close(ts->connfd);
      break;
    }
    for (int i = 0; i < len; ++i) buf[i] = toupper(buf[i]);
    int wlen = write(ts->connfd, buf, len);
    write(STDOUT_FILENO, buf, wlen);
    if (wlen < 0) {
      perror("write()");
      exit(1);
    }
  }
  printf("child close %ld\n", pthread_self());
  return (void *)0;
}

int main() {
  struct s_info ts[256];
  pthread_t tid;
  int i = 0;
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd == -1) {
    perror("socket()");
    exit(1);
  }

  struct sockaddr_in serv_addr, clie_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int be = bind(lfd, (sockaddr *)&(serv_addr), sizeof(serv_addr));
  if (be == -1) {
    perror("bind()");
    exit(1);
  }
  int le = listen(lfd, SOMAXCONN);
  if (le == -1) {
    perror("listen()");
    exit(1);
  }

  while (1) {
    printf("Parent IO %ld\n", pthread_self());
    socklen_t clie_addr_len = sizeof(clie_addr);
    char clie_ip[BUFSIZ];
    int cfd = accept(lfd, (sockaddr *)&clie_addr, &clie_addr_len);
    if (cfd == -1) {
      perror("accept()");
      exit(1);
    }
    printf("client ip = %s, client port = %d \n",
           inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_ip,
                     sizeof(clie_ip)),
           ntohs(clie_addr.sin_port));
    ts[i].cliaddr = clie_addr;
    ts[i].connfd = cfd;
    pthread_create(&tid, NULL, do_worker, (void *)&ts[i]);
    pthread_detach(tid);
    ++i;
  }
  return 0;
}