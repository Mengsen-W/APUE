/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 14:51:46
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 15:15:14
 * @Description: signal 初步尝试
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void int_handler(int s) { write(1, "!", 1); }

int main() {
  signal(SIGINT, int_handler);
  for (int i = 0; i < 10; i++) {
    if (write(1, "**", 2)) sleep(1);
  }
  return 0;
}