/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-13 11:10:57
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 11:35:36
 * @Description: epoll 监听管道
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXLINE 10

int main(int argc, char* argv[]) {
  int efd, i;
  int pfd[2];
  pid_t pid;
  char buf[MAXLINE];
  char ch = 'a';

  pipe(pfd);
  pid = fork();

  if (pid == 0) { /* child processes */
    close(pfd[0]);
    while (1) {
      // aaaa\n
      for (i = 0; i < MAXLINE / 2; ++i) buf[i] = ch;
      buf[i - 1] = '\n';
      ++ch;
      // bbbb\n
      for (; i < MAXLINE; ++i) buf[i] = ch;
      buf[i - 1] = '\n';
      ++ch;
      write(pfd[1], buf, sizeof(buf));
      sleep(5);
    }
    close(pfd[1]);
  }

  if (pid > 0) { /*parents processes*/
    struct epoll_event events;
    struct epoll_event resevent[10];

    close(pfd[1]);
    efd = epoll_create(10);

    events.events = EPOLLIN | EPOLLET;  // 边缘触发
    // events.events = EPOLLIN;  // 水平触发（默认）
    events.data.fd = pfd[0];
    epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &events);

    while (1) { /* 写完才会被触发没被触发就会阻塞 */
      int res = epoll_wait(efd, resevent, 10, -1);
      printf("res = %d\n", res);
      if (resevent[0].data.fd == pfd[0]) {
        int len = read(pfd[0], buf, MAXLINE / 2);  // 一次只读5个读不完
        write(STDOUT_FILENO, buf, len);
      }
    }
  }
  return 0;
}