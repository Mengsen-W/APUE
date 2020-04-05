/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-05 08:48:25
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 08:57:38
 * @Description: posixthread 基础
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* func(void* p) {
  puts("Thread is working!");
  pthread_exit(NULL);
  // return 0;
}

int main() {
  pthread_t tid;
  int err;
  puts("Begin");

  err = pthread_create(&tid, NULL, func, NULL);
  if (err) {
    fprintf(stderr, "pthread_creat() error %s\n", strerror(err));
    exit(1);
  }
  pthread_join(tid, NULL);
  puts("End");

  return 0;
}