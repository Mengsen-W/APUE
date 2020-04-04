/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 16:21:05
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 16:36:27
 * @Description: 定时5秒累加多次
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static volatile int loop = 1;

static void alrm_handler(int s) { loop = 0; }

int main() {
  long long int count = 0;
  alarm(5);
  signal(SIGALRM, alrm_handler);
  while (loop) {
    ++count;
  }
  printf("%lld\n", count);
  return 0;
}