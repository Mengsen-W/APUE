/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-10 12:12:21
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-10 12:24:55
 * @Description: 信号量 -- 若数目不够则等待
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const static int NUM = 5;
static int queue[NUM];
static sem_t blank_number, product_number;
// 两个信号量会产生顺序，防止死锁
// 若只有一个信号那么消费者进入后可能产生死锁

void* product(void* arg) {
  int i = 0;
  while (1) {
    sem_wait(&blank_number);  // --空白
    queue[i] = rand() % 1000 + 1;
    printf("---------Produce = %d ------------\n", queue[i]);
    sem_post(&product_number);  // ++产品

    i = (i + 1) / NUM;  // 借助下标实现环形
    sleep(rand() % 1);
  }
}

void* consumer(void* arg) {
  int i = 0;
  while (1) {
    sem_wait(&product_number);  // --产品
    printf("---------Consumer = %d ------------\n", queue[i]);
    queue[i] = 0;
    sem_post(&blank_number);  // ++空白

    i = (i + 1) / NUM;  // 借助下标实现环形
    sleep(rand() % 1);
  }
}

int main() {
  pthread_t pid, cid;
  sem_init(&blank_number, 0, NUM);  //初始化空格子为5
  sem_init(&product_number, 0, 0);

  pthread_create(&pid, nullptr, product, nullptr);
  pthread_create(&cid, nullptr, consumer, nullptr);

  pthread_join(pid, nullptr);
  pthread_join(cid, nullptr);

  sem_destroy(&blank_number);
  sem_destroy(&product_number);

  return 0;
}
