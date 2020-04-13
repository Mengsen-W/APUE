/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-12 12:40:54
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 10:53:26
 * @Description: select server
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int SERV_PORT = 6666;

int main(int argc, char** argv) {
  int i = 0, j = 0, n = 0, maxi = 0;

  int nready, client[FD_SETSIZE];
  int maxfd, listenfd, connfd, sockfd;
  char buf[BUFSIZ], str[INET_ADDRSTRLEN];

  struct sockaddr_in clie_addr, serv_addr;
  socklen_t clie_addr_len;
  fd_set rset, allset;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("socket()");
    exit(1);
  }

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERV_PORT);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int be = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if (be == -1) {
    perror("bind()");
    exit(1);
  }

  int le = listen(listenfd, SOMAXCONN);
  if (le == -1) {
    perror("listen()");
    exit(1);
  }

  maxfd = listenfd;
  maxi = -1;                                            // 表示数组下标
  for (int i = 0; i < FD_SETSIZE; i++) client[i] = -1;  // 初始化 client[]
  FD_ZERO(&allset);
  FD_SET(listenfd, &allset);

  while (1) {
    rset = allset;
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
    if (nready < 0) {
      perror("select()");
      exit(1);
    }
    if (FD_ISSET(listenfd, &rset)) {  // 只能一个一个处理
      clie_addr_len = sizeof(clie_addr);
      // 此时accept不会阻塞了，因为已经有客户端进入了
      connfd = accept(listenfd, (struct sockaddr*)&clie_addr, &clie_addr_len);
      printf("received from %s at PORT %d \n",
             inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)),
             ntohs(clie_addr.sin_port));
      for (i = 0; i < FD_SETSIZE; ++i) {
        if (client[i] < 0) {
          client[i] = connfd;  // 找到空位赋值
          break;
        }
      }

      if (i == FD_SETSIZE) {
        fputs("too many client connections\n", stderr);
        exit(1);
      }

      FD_SET(connfd, &allset);             // 加入文件描述符
      if (connfd > maxfd) maxfd = connfd;  // 更新maxfd
      if (i > maxi) maxi = i;              // 更新数组上限指针
      if (--nready ==
          0)  //对应只有一个connfd返回的情况，同时也可以把listenfd减掉保证进入下面的是通讯
        continue;  // 如果多个进入则说明要处理的只有listenfd还有别的，就继续向下处理
    }  // 对应多个客户端同时进入，但是也只是返回了1个listedfd，此处可继续accept
    printf("complete accept\n");
    for (i = 0; i <= maxi; ++i) {  // 检测哪个clients有数据就绪

      if ((sockfd = client[i]) < 0) continue;

      if (FD_ISSET(sockfd, &rset)) {
        printf("complete ISSET\n");
        if ((n = read(sockfd, buf, sizeof(buf))) == 0) {  //是否关闭client
          printf("close()\n");
          close(sockfd);            //清理操作
          FD_CLR(sockfd, &allset);  // 客户端关闭也会导致产生read事件
          client[i] = -1;
        } else if (n > 0) {
          for (j = 0; j < n; ++j) buf[j] = toupper(buf[j]);  // 实际业务
          printf("toupper\n");
          int we = write(STDOUT_FILENO, buf, n);
          if (we == -1) {
            perror("write");
          }
          we = write(sockfd, buf, n);
          if (we == -1) {
            perror("write");
          }
        }

        if (--nready == 0) break;
      }
    }
  }
  close(listenfd);
  return 0;
}
