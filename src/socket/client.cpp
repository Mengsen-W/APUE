/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-11 11:31:32
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-12 21:57:00
 * @Description: 服务端
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
int main() {
  printf("hello socket I'm client\n");
  int cfd = socket(AF_INET, SOCK_STREAM, 0);

  // 隐式绑定
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
  connect(cfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

  char buf[BUFSIZ];
  while (1) {
    // char* s = fgets(buf, BUFSIZ, stdin);
    // if (s == NULL) {
    //   perror("fgets()");
    //   exit(1);
    // }
    int wlen = write(cfd, "a\n", 2);
    if (wlen < 0) {
      perror("write()");
      exit(1);
    }
    int len = read(cfd, buf, sizeof(buf));
    if (len < 0) {
      perror("read()");
      exit(1);
    }

    wlen = write(STDOUT_FILENO, buf, len);
    if (wlen < 0) {
      perror("write()");
      exit(1);
    }
  }
  close(cfd);

  return 0;
}