/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 14:51:46
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 20:48:43
 * @Description: signal 信号集
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

static void int_handler(int s) { int e = write(1, "!", 1); }

int main() {
  signal(SIGINT, int_handler);
  sigset_t set, savesets;
  //保证全空，确定当前全局状态不会改变
  // 保存和恢复
  sigprocmask(SIG_UNBLOCK, &set, &savesets);
  sigemptyset(&set);
  // 阐述对哪个信号要block
  sigaddset(&set, SIGINT);
  for (int j = 0; j < 1000; ++j) {
    sigprocmask(SIG_BLOCK, &set, NULL);
    for (int i = 0; i < 5; i++) {
      int e = write(1, "*", 1);
      sleep(1);
    }
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    write(1, "\n", 1);
  }
  sigprocmask(SIG_SETMASK, &savesets, NULL);
  return 0;
}