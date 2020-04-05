/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-05 09:34:35
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 13:29:08
 * @Description: 多线程 线程池 求质数
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

#define LEFT 0
#define RIGHT 20000
#define THRNUM 2

static int num = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void* thr_prime(void* ptr) {
  while (1) {
    int i = 0;
    pthread_mutex_lock(&mutex);
    while (num == 0) {
      pthread_mutex_unlock(&mutex);
      sched_yield();
      pthread_mutex_lock(&mutex);
    }
    if (num == -1) {
      pthread_mutex_unlock(&mutex);
      break;
    }
    i = num;
    num = 0;
    pthread_mutex_unlock(&mutex);
    int mark = 1;
    int size = i / 2;
    for (int j = 2; j < size; ++j) {
      if (i % j == 0) {
        mark = 0;
        break;
      }
    }
    if (mark) printf(" %d is a paimer\n", i);
  }
  pthread_exit(ptr);
}

int main() {
  pthread_t tid[THRNUM];
  for (int i = 0; i < THRNUM; ++i) {
    int err = pthread_create(tid + i, NULL, thr_prime, NULL);
    if (err) {
      fprintf(stderr, "pthread_creat(): %s\n", strerror(err));
      exit(1);
    }
  }

  for (int i = LEFT; i <= RIGHT; ++i) {
    pthread_mutex_lock(&mutex);
    while (num != 0) {
      pthread_mutex_unlock(&mutex);
      sched_yield();
      pthread_mutex_lock(&mutex);
    }
    num = i;
    pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_lock(&mutex);
  while (num != 0) {
    pthread_mutex_unlock(&mutex);
    sched_yield();
    pthread_mutex_lock(&mutex);
  }
  num = -1;
  pthread_mutex_unlock(&mutex);

  for (int i = LEFT; i < THRNUM; ++i) {
    // pthread_join(tid[i - LEFT], NULL);
    pthread_join(tid[i], nullptr);
  }
  pthread_mutex_destroy(&mutex);
  return 0;
}