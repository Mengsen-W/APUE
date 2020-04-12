/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-11 10:27:05
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-11 16:58:32
 * @Description: 服务端
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERV_PORT 6666

int main() {
  printf("hello socket. I'm server.\n");
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd == -1) {
    perror("socket()");
    exit(1);
  }
  int opt = 1;
  setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in serv_addr, clie_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
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

  socklen_t clie_addr_len = sizeof(clie_addr);
  char clie_ip[BUFSIZ];
  int cfd = accept(lfd, (sockaddr *)&clie_addr, &clie_addr_len);
  if (cfd == -1) {
    perror("accept()");
    exit(1);
  }
  printf(
      "client ip = %s, client port = %d \n",
      inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_ip, sizeof(clie_ip)),
      ntohs(clie_addr.sin_port));

  char buf[BUFSIZ];
  while (1) {
    int len = read(cfd, buf, sizeof(buf));
    for (int i = 0; i < len; ++i) {
      buf[i] = toupper(buf[i]);
    }
    int wlen = write(cfd, buf, len);
    if (wlen < 0) {
      perror("write()");
      exit(1);
    }
  }
  close(cfd);
  close(lfd);
  return 0;
}