/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-08 10:07:00
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-11 17:16:32
 * @Description: SIGCHID
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void do_sig_child(int signo) {
  printf("system call !!!");
  int status;
  pid_t pid;

  // if ((pid = waitpid(0, &status, WNOHANG)) > 0) {
  // 防止执行函数的过程中多个信号进入导致最后只有一个子进程被回收了
  while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
    if (WIFEXITED(status))
      printf("------------------ child %d exit %d\n", pid, WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
      printf("------------------ child %d signal %d\n", pid, WTERMSIG(status));
  }
}

int main() {
  int i;
  pid_t pid;
  for (i = 0; i < 10; ++i) {
    if ((pid = fork()) == 0) {
      break;
    } else if (pid < 0) {
      perror("fork()");
      exit(1);
    }
  }

  if (pid == 0) {
    int n = 1;
    while (n--) {
      printf("child ID %d\n", getpid());
      sleep(1);
    }
    return i + 1;

  } else if (pid > 1) {
    struct sigaction act;

    act.sa_handler = do_sig_child;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigaction(SIGCHLD, &act, nullptr);

    while (1) {
      printf("Parent IO %d\n", getpid());
      sleep(1);
    }
  }

  return 0;
}