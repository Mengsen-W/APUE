/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 17:08:17
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 20:54:02
 * @Description: 多线程条件通知令牌桶
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmath>
#include <iostream>

struct mytbf_st {
  int cps;
  int burst;
  int token;
  int pos;
  pthread_mutex_t mut;
  pthread_cond_t cond;
};
// 每秒传输字节数
#define CPS 10
// 最大令牌数
#define BURST 20
// 流的buffer
#define BUFSIZE 1024

// 库中文件令牌桶数组的最大值
#define MYTBF_MAX 1024

static pthread_mutex_t mut_job = PTHREAD_MUTEX_INITIALIZER;

static struct mytbf_st* job[MYTBF_MAX];
static pthread_t tid_alarms;
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

static void* thr_alarm(void* s) {
  while (1) {
    pthread_mutex_lock(&mut_job);
    for (int i = 0; i < MYTBF_MAX; ++i) {
      if (job[i] != NULL) {
        pthread_mutex_lock(&job[i]->mut);
        job[i]->token += job[i]->cps;
        if (job[i]->token > job[i]->burst) job[i]->token = job[i]->burst;
        pthread_cond_broadcast(&job[i]->cond);
        pthread_mutex_unlock(&job[i]->mut);
      }
    }
    pthread_mutex_unlock(&mut_job);
    sleep(1);
  }
}

int mytbf_destroy(struct mytbf_st* ptr) {
  pthread_mutex_lock(&mut_job);
  job[ptr->pos] = nullptr;
  pthread_mutex_unlock(&mut_job);
  pthread_mutex_destroy(&ptr->mut);
  pthread_cond_destroy(&ptr->cond);
  free(ptr);
  return 0;
}

static void module_unload() {
  pthread_cancel(tid_alarms);
  pthread_join(tid_alarms, NULL);

  for (int i = 0; i < MYTBF_MAX; ++i) {
    if (job[i] != NULL) {
      mytbf_destroy(job[i]);
    }
  }
  pthread_mutex_destroy(&mut_job);
}

static void module_load() {
  int err = pthread_create(&tid_alarms, NULL, thr_alarm, NULL);
  if (err) {
    fprintf(stderr, "pthread creat(): %s\n", strerror(err));
    exit(1);
  }
  atexit(module_unload);
}

static int get_free_pos_unlocked(void) {
  for (int i = 0; i < MYTBF_MAX; ++i) {
    if (job[i] == nullptr) return i;
  }
  return -1;
}

static int min(int a, int b) {
  if (a > b)
    return b;
  else
    return a;
}

struct mytbf_st* mytbf_init(int cps, int burst) {
  struct mytbf_st* me;

  pthread_once(&init_once, module_load);

  me = (mytbf_st*)malloc(sizeof(*me));
  if (me == nullptr) {
    return nullptr;
  }
  me->token = 0;
  me->cps = cps;
  me->burst = burst;
  pthread_mutex_init(&me->mut, nullptr);
  pthread_cond_init(&me->cond, nullptr);

  pthread_mutex_lock(&mut_job);
  int pos = get_free_pos_unlocked();
  if (pos < 0) {
    pthread_mutex_unlock(&mut_job);
    return nullptr;
  }
  me->pos = pos;
  job[pos] = me;
  pthread_mutex_unlock(&mut_job);

  return me;
}

int mytbf_fetchtoken(struct mytbf_st* ptr, int size) {
  if (size <= 0) return -EINVAL;

  pthread_mutex_lock(&ptr->mut);
  while (ptr->token <= 0) {
    pthread_cond_wait(&ptr->cond, &ptr->mut);
  }
  int n = min(ptr->token, size);
  ptr->token -= n;
  pthread_mutex_unlock(&ptr->mut);

  return n;
}

int mytbf_returntoken(struct mytbf_st* ptr, int size) {
  if (size <= 0) return -EINVAL;

  pthread_mutex_lock(&ptr->mut);
  ptr->token += size;
  if (ptr->token > ptr->burst) ptr->token = ptr->burst;
  pthread_cond_broadcast(&ptr->cond);
  pthread_mutex_unlock(&ptr->mut);

  return size;
}

int main(int argc, char** argv) {
  int sfd, dfd = 1;
  char buf[BUFSIZE];
  int len = 0, ret = 0, pos = 0;

  mytbf_st* tbf;
  if (argc < 2) {
    fprintf(stderr, "Usage ... \n");
    exit(1);
  }

  tbf = mytbf_init(CPS, BURST);
  if (tbf == nullptr) {
    fprintf(stderr, "error init\n");
    exit(1);
  }
  do {
    sfd = open(argv[1], O_RDONLY);
    if (sfd < 0) {
      if (errno != EINTR) {
        perror("open()");
        exit(1);
      }
    }
    // 若文件打开失败就一直打开
  } while (sfd < 0);

  while (1) {
    int size = mytbf_fetchtoken(tbf, BUFSIZE);
    if (size < 0) {
      fprintf(stderr, "%s", strerror(-size));
      exit(1);
    }

    while ((len = read(sfd, buf, size)) < 0) {
      // 若文件读时遇到了超时中断则 read不会成功而且会返回 EINTR
      if (errno == EINTR) continue;
      //其他错误
      perror("read()");
      break;
    }
    // 读到文件尾
    while (len == 0) break;

    // 剩余了token
    if (size - len > 0) mytbf_returntoken(tbf, size - len);

    pos = 0;
    while (len > 0) {
      // 防止储存满无法一次写完
      ret = write(dfd, buf + pos, len);
      if (ret < 0) {
        if (errno == EINTR) continue;
        perror("write()");
        exit(1);
      }
      pos += ret;
      len -= ret;
    }
  }
  close(sfd);
  mytbf_destroy(tbf);
  return 0;
}
