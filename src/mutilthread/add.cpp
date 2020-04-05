/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-05 10:53:12
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 11:48:23
 * @Description: 互斥竞争
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ctime>
#include <iostream>

static const int THRNUM = 2;
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

static void* thr_add(void* p) {
  pthread_mutex_lock(&mut);
  // 每次原地修改
  time_t now = time(NULL);
  sleep(1);
  std::cout << time(&now) << std::endl;
  pthread_mutex_unlock(&mut);
  pthread_exit(NULL);
}

int main() {
  pthread_t tid[10];
  for (int i = 0; i < 10; i++) {
    int err = pthread_create(&tid[i], NULL, thr_add, NULL);
    if (err) {
      fprintf(stderr, "pthread_creat(): %s\n", strerror(err));
      exit(1);
    }
  }
  for (int i = 0; i < 10; i++) {
    pthread_join(tid[i], NULL);
  }
  pthread_mutex_destroy(&mut);
  return 0;
}