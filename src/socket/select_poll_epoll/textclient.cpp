/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-13 11:50:26
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 11:57:56
 * @Description: 测试用客户端
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERV_PORT 6666
#define MAXLINE 10

int main() {
  printf("hello socket I'm client\n");
  int cfd = socket(AF_INET, SOCK_STREAM, 0);
  char ch = 'a';
  int i = 0;

  // 隐式绑定
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
  connect(cfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  char buf[MAXLINE];

  while (1) {
    for (i = 0; i < MAXLINE / 2; ++i) buf[i] = ch;
    buf[i - 1] = '\n';
    ++ch;
    for (; i < MAXLINE; ++i) buf[i] = ch;
    buf[i - 1] = '\n';
    ++ch;
    int wlen = write(cfd, buf, MAXLINE);
    sleep(5);
    if (wlen < 0) {
      perror("write()");
      exit(1);
    }
  }
  close(cfd);

  return 0;
}