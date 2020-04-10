/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-10 10:31:50
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-10 11:04:56
 * @Description: 读写锁 读共享写独占，写锁优先级高
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

/*
 * @ 三个线程对资源写，五个线程对资源读
 */

static int counter;
pthread_rwlock_t rwlock;

void* th_write(void* arg) {
  int t;
  int i = (long int)arg;

  while (1) {
    t = counter;
    usleep(1000);

    pthread_rwlock_wrlock(&rwlock);
    printf("============ write %d: %lu: counter = %d ++counter = %d\n", i,
           pthread_self(), t, ++counter);
    pthread_rwlock_unlock(&rwlock);

    usleep(5000);
  }
  return nullptr;
}

void* th_read(void* arg) {
  int i = (long int)arg;

  while (1) {
    usleep(1000);

    pthread_rwlock_rdlock(&rwlock);
    printf("---------------- read %d: %lu: counter = %d\n", i, pthread_self(),
           counter);
    pthread_rwlock_unlock(&rwlock);

    usleep(900);
  }
  return nullptr;
}

int main(void) {
  int i;
  pthread_t tid[8];

  pthread_rwlock_init(&rwlock, nullptr);

  for (i = 0; i < 3; ++i) pthread_create(&tid[i], nullptr, th_write, (void*)i);
  for (i = 0; i < 5; ++i)
    pthread_create(&tid[i + 3], nullptr, th_read, (void*)i);
  for (i = 0; i < 8; ++i) pthread_join(tid[i], nullptr);

  return 0;
}