/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-11 16:51:29
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-11 17:44:00
 * @Description: 多进程服务器
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERV_PORT 8888

static void wait_child(int signo) {
  printf("system call !!!");
  int status;
  pid_t pid;

  while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
    if (WIFEXITED(status))
      printf("------------------ child %d exit %d\n", pid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      printf("------------------ child %d signal %d\n", pid, WTERMSIG(status));
  }
  return;
}
int main() {
  int lfd, cfd;
  pid_t pid;
  lfd = socket(AF_INET, SOCK_STREAM, 0);
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
    socklen_t clie_addr_len = sizeof(clie_addr);
    char clie_ip[BUFSIZ];
    cfd = accept(lfd, (sockaddr *)&clie_addr, &clie_addr_len);
    if (cfd == -1) {
      perror("accept()");
      exit(1);
    }
    printf("client ip = %s, client port = %d \n",
           inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_ip,
                     sizeof(clie_ip)),
           ntohs(clie_addr.sin_port));
    printf("Parent IO %d\n", getpid());
    pid = fork();
    if (pid < 0) {
      perror("fork");
      exit(1);
    } else if (pid == 0) {
      close(lfd);
      break;
    } else {
      close(cfd);
      signal(SIGCHLD, wait_child);
    }
  }

  if (pid == 0) {
    printf("child ID %d\n", getpid());
    char buf[BUFSIZ];
    while (1) {
      int len = read(cfd, buf, sizeof(buf));
      if (len == 0) {
        close(cfd);
        return 0;
      }
      for (int i = 0; i < len; ++i) buf[i] = toupper(buf[i]);
      int wlen = write(cfd, buf, len);
      write(STDOUT_FILENO, buf, wlen);
      if (wlen < 0) {
        perror("write()");
        exit(1);
      }
    }
  }

  return 0;
}