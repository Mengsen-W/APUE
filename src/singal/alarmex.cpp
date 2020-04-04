/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 16:09:08
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 16:19:33
 * @Description: alarm 的基本用法
 */

#include <unistd.h>

#include <cstdio>
#include <cstdlib>

int main() {
  // 谁在最下面听谁的
  // alarm(10);
  // alarm(1);
  alarm(5);
  while (1) {
    // 自我陷入，而是等待一个信号打断
    pause();
  }
  return 0;
}
