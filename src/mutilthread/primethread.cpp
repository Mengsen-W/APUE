/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-05 09:34:35
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 12:55:34
 * @Description: 多线程 求质数
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LEFT 1000000
#define RIGHT 1000200
#define THRNUM (RIGHT - LEFT + 1)

static void* thr_prime(void* ptr) {
  // 当 i 未执行到这条语句时，有可能i的值已经改变了
  int i = *(int*)ptr;
  int mark = 1;
  for (int j = 2; j < i / 2; j++) {
    if (i % j == 0) {
      mark = 0;
      break;
    }
  }
  if (mark) printf("%d is a paimer\n", i);
  //将地址传出
  pthread_exit(ptr);
  return 0;
}

int main() {
  pthread_t tid[THRNUM];
  int* ptr;
  for (int i = LEFT; i <= RIGHT; ++i) {
    int* ptr = new int(i);
    int err = pthread_create(tid + (i - LEFT), NULL, thr_prime, ptr);
    if (err) {
      fprintf(stderr, "pthread_creat(): %s\n", strerror(err));
      exit(1);
    }
  }

  for (int i = LEFT; i <= RIGHT; ++i) {
    // pthread_join(tid[i - LEFT], NULL);
    pthread_join(tid[i - LEFT], (void**)&ptr);
    delete ptr;
  }

  return 0;
}