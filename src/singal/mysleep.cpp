/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-07 14:51:42
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-07 16:17:22
 * @Description: 实现自己的 sleep
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void catch_signal(int signo) {}

static unsigned int mysleep(unsigned int seconds) {
  struct sigaction act, oact;

  act.sa_handler = catch_signal;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGALRM, &act, &oact);

  alarm(seconds);
  // 因为操作非原子
  // 可能在此间由于优先级较高
  // 阻断了更长时间，导致 alarm 失效
  // 回到此进程后先调用回调函数
  // 再次进入pause后面再也不能被唤醒了
  pause();
  // 可以用sigsuspend
  // 先在外面屏蔽关键字，只有在sigsuspend内才不会被屏蔽

  alarm(0);
  sigaction(SIGALRM, &oact, NULL);

  return 0;
}

int main() {
  printf("------------------\n");
  while (1) {
    printf("------------------\n");
    mysleep(3);
    printf("------------------\n");
  }
  return 0;
}