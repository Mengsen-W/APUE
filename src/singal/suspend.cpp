/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 14:51:46
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 21:35:31
 * @Description: signal suspend
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

static void int_handler(int s) { int e = write(1, "!", 1); }

int main() {
  signal(SIGINT, int_handler);
  sigset_t set, savesets, oset;
  //保证全空，确定当前全局状态不会改变
  sigemptyset(&set);
  // 阐述对哪个信号要block
  sigaddset(&set, SIGINT);
  // 保存和恢复
  sigprocmask(SIG_UNBLOCK, &set, &savesets);
  sigprocmask(SIG_BLOCK, &set, &oset);
  for (int j = 0; j < 1000; ++j) {
    for (int i = 0; i < 5; i++) {
      int e = write(1, "*", 1);
      sleep(1);
    }
    write(1, "\n", 1);
    // 恢复解除阻塞，并马上进入等待状态
    // 这样就不会再解除阻塞到pause之间响应信号
    // 这一个信号是对于整个操作的
    //在这里是打印！并且继续执行
    //若用之前的会只打印！，在pause住
    // 因为不是原子操作
    sigsuspend(&oset);
    //调换顺序导致死锁
    /* 这是一个supend
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    ! 此时响应信号 不是原子操作
    pause();
    ! 恢复信号
    */
  }
  sigprocmask(SIG_SETMASK, &savesets, NULL);
  return 0;
}