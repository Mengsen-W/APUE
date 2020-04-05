/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-05 08:56:59
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 09:23:31
 * @Description: 测试 cleanup
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

static void cleanup_func(void* p) { std::cout << "hock start " << std::endl; }

static void* func(void* p) {
  puts("Thread is working!");
  pthread_cleanup_push(cleanup_func, NULL);
  pthread_cleanup_push(cleanup_func, NULL);
  pthread_cleanup_push(cleanup_func, NULL);
  puts("Over");
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(0);
  pthread_cleanup_pop(1);
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