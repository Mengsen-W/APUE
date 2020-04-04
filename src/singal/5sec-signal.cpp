/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 16:21:05
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 16:50:23
 * @Description: 定时5秒累加多次
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static volatile int loop = 1;

static void alrm_handler(int s) {
  // alarm 链
  alarm(1);
  loop = 0;
}

int main() {
  long long int count = 0;
  signal(SIGALRM, alrm_handler);
  alarm(1);
  while (1) {
    while (loop) {
      ++count;
    }
    if (loop == 0) {
      printf("%lld\n", count);
      loop = 1;
    }
  }
  return 0;
}