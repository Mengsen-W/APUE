/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-06 14:52:31
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-06 15:58:17
 * @Description: 实现进程内管道
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PIPESIZE 4096
#define MYPIPE_READ 0x00000001UL
#define MYPIPE_WRITE 0x00000002UL

struct mypipe_st {
  int head;
  int tail;
  char data[PIPESIZE];
  int dataSize;
  int count_rd;
  int count_wr;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
};

mypipe_st *mypipe_init(void) {
  struct mypipe_st *me;
  me = (struct mypipe_st *)malloc(sizeof(struct mypipe_st));
  if (me == nullptr) return nullptr;
  me->head = 0;
  me->tail = 0;
  me->dataSize = 0;
  me->count_rd = 0;
  me->count_wr = 0;
  pthread_mutex_init(&me->mutex, nullptr);
  pthread_cond_init(&me->cond, nullptr);
  return me;
}

int mypipe_destroy(struct mypipe_st *me) {
  pthread_mutex_destroy(&me->mutex);
  pthread_cond_destroy(&me->cond);
  free(me);
  return 0;
}

int mypipe_register(mypipe_st *me, int opmap) {
  pthread_mutex_lock(&me->mutex);
  if (opmap & MYPIPE_READ) me->count_rd++;
  if (opmap & MYPIPE_WRITE) me->count_wr++;

  pthread_cond_broadcast(&me->cond);

  while (me->count_rd <= 0 || me->count_wr <= 0) {
    pthread_cond_wait(&me->cond, &me->mutex);
  }

  pthread_mutex_unlock(&me->mutex);

  return 0;
}
int mypipe_unregister(mypipe_st *me, int opmap) {
  pthread_mutex_lock(&me->mutex);

  if (opmap & MYPIPE_READ) me->count_rd--;
  if (opmap & MYPIPE_WRITE) me->count_wr--;

  pthread_cond_broadcast(&me->cond);
  pthread_mutex_unlock(&me->mutex);
  return 0;
}

int next(int) {}

static int mypipe_read_bytes_unlock(struct mypipe_st *me, char *buffer) {
  if (me->dataSize <= 0) return -1;
  *buffer = me->data[me->head];
  me->head = next(me->head);
  me->dataSize--;
  return 0;
}

int mypipe_read(struct mypipe_st *me, char *buf, size_t count) {
  int i = 0;
  pthread_mutex_lock(&me->mutex);

  while (me->dataSize <= 0 && me->count_wr > 0)
    pthread_cond_wait(&me->cond, &me->mutex);
  if (me->dataSize <= 0 && me->count_wr <= 0) {
    pthread_mutex_unlock(&me->mutex);
    return 0;
  }
  for (i = 0; i < count; ++i) {
    if (mypipe_read_bytes_unlock(me, buf + i) != 0) break;
  }
  pthread_cond_broadcast(&me->cond);
  pthread_mutex_unlock(&me->mutex);
  return i;
}
int mypipe_write(struct mypipe_st *, const void *, size_t) {}
