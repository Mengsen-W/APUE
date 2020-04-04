/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 17:08:17
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 22:41:54
 * @Description: 令牌桶 + sigaction
 */
//! 系统调用会陷入内核态
//! shell 在用户态

#include <errno.h>
#include <fcntl.h>
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
};
// 每秒传输字节数
#define CPS 10
// 最大令牌数
#define BURST 20
// 流的buffer
#define BUFSIZE 1024

// 库中文件令牌桶数组的最大值
#define MYTBF_MAX 1024

static struct mytbf_st* job[MYTBF_MAX];
static int inited = 0;
static struct sigaction alrm_sa_save;

static void alarm_action(int s, siginfo_t* infop, void* unused) {
  if (infop->si_code == SI_KERNEL)
    for (int i = 0; i < MYTBF_MAX; ++i) {
      if (job[i] != NULL) {
        job[i]->token += job[i]->cps;
        if (job[i]->token > job[i]->burst) job[i]->token = job[i]->burst;
      }
    }
}

static void module_unload() {
  // 恢复状态
  sigaction(SIGALRM, &alrm_sa_save, nullptr);

  struct itimerval itv;
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = 0;
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = 0;

  setitimer(ITIMER_REAL, &itv, nullptr);

  for (int i = 0; i < MYTBF_MAX; ++i) {
    free(job[i]);
  }
}

static void module_load() {
  struct sigaction sa;
  sa.sa_sigaction = alarm_action;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGALRM, &sa, &alrm_sa_save);

  struct itimerval itv;
  itv.it_interval.tv_sec = 1;
  itv.it_interval.tv_usec = 0;
  itv.it_value.tv_sec = 1;
  itv.it_value.tv_usec = 0;

  setitimer(ITIMER_REAL, &itv, nullptr);
  atexit(module_unload);
}

static int get_free_pos(void) {
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
  if (!inited) {
    module_load();
  }
  me = (mytbf_st*)malloc(sizeof(*me));
  int pos = get_free_pos();
  if (pos < 0) {
    return nullptr;
  }
  if (me == nullptr) {
    return nullptr;
  }
  me->token = 0;
  me->cps = cps;
  me->burst = burst;
  me->pos = pos;
  job[pos] = me;
  return me;
}

int mytbf_fetchtoken(struct mytbf_st* ptr, int size) {
  if (size <= 0) return -EINVAL;
  while (ptr->token <= 0) {
    // ptr->token += size;
    pause();
  }
  int n = min(ptr->token, size);
  ptr->token -= n;
  return n;
}

int mytbf_returntoken(struct mytbf_st* ptr, int size) {
  std::cout << "returntoken" << std::endl;
  if (size <= 0) return -EINVAL;
  ptr->token += size;
  if (ptr->token > ptr->burst) ptr->token = ptr->burst;
  return size;
}

int mytbf_destroy(struct mytbf_st* ptr) {
  job[ptr->pos] = nullptr;
  free(ptr);
  return 0;
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
    if (len == 0) break;

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
