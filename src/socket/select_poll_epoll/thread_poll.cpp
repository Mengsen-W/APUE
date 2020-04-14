/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-14 14:10:20
 * @Last Modified by:   Mengsen.Wang
 * @Last Modified time: 2020-04-14 14:10:20
 * @Description: 线程池
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_TIME 10        /* 10s 检测一次 */
#define MIN_WAIT_TASK_NUM 10   /* 最小等待线程数 */
#define DEFAULT_THREAD_VARY 10 /* 销毁创建线程步长 */
#define true 1
#define false 0

typedef struct {
  void *(*function)(void *); /* 回调函数 */
  void *arg;                 /* 参数 */
} threadpool_task_t;

/* 描述线程池相关信息 */
struct threadpool_t {
  pthread_mutex_t lock;           /* 锁住本结构体 */
  pthread_mutex_t thread_counter; /* 记录忙状态的锁 */
  pthread_cond_t queue_not_full;  /* 任务满时通知信号 */
  pthread_cond_t queue_not_empty; /* 不为空的通知信号 */

  pthread_t *threads;   /* 存放线程池中的每个线程tid 数组*/
  pthread_t adjust_tid; /* 管理线程池的线程id */
  threadpool_task_t *task_queue; /* 任务队列数组 */

  int min_thr_num;       /* 线程池的最小线程数 */
  int max_thr_num;       /* 线程池的最大任务数 */
  int live_thr_num;      /* 当前存活线程个数 */
  int busy_thr_num;      /* 忙线程个数 */
  int wait_exit_thr_num; /* 即将销毁线程数 */

  int queue_front;    /* task_queue 队头下标 */
  int queue_rear;     /* task_queue 队尾下标 */
  int queue_size;     /* task_queue 实际任务数 */
  int queue_max_size; /* task_queue 最大任务数 */

  int shutdown; /* 线程池关闭标志位 */
};

/* 创建线程 */
threadpool_t *threadpool_create(int min_thr_num, int max_thr_num,
                                int queue_max_size);
/* 实际线程调用函数 */
void *threadpool_thread(void *threadpool);
/* 增加任务 */
void *threadpool_add(threadpool_t *pool, void *(*function)(void *arg),
                     void *arg);
/* 管理者线程 */
void *adjust_thread(void *threadpool);
/* 销毁线程池 */
int threadpool_destroy(threadpool_t *pool);
/* 清理内存 */
int threadpool_free(threadpool_t *pool);
/* 统计线程池全部线程数 */
int threadpool_all_threadnum(threadpool_t *pool);
/* 统计忙线程数 */
int threadpool_busy_threadnum(threadpool_t *pool);
/* 判断线程是否存活 */
bool is_thread_alive(pthread_t tid);

/* 测试 */
#if 1
/* 线程池中的线程，模拟处理业务 */
void *process(void *arg) {
  printf("thread 0x%x working in task %ld\n", (unsigned int)pthread_self(),
         (long int)arg);
  sleep(1);
  printf("task %ld is end\n", (long int)arg);

  return nullptr;
}

int main() {
  threadpool_t *thp = threadpool_create(3, 100, 100);
  printf("poll inited\n");

  int *num = (int *)malloc(sizeof(int) * 20);
  int i;
  for (i = 0; i < 20; ++i) {
    num[i] = i;
    printf("add task %d\n", i);
    threadpool_add(thp, process, (void *)&num[i]);
  }
  sleep(10);
  threadpool_destroy(thp);

  return 0;
}
#endif

threadpool_t *threadpool_create(int min_thr_num, int max_thr_num,
                                int queue_max_size) {
  int i;
  threadpool_t *pool = nullptr;
  do {
    if ((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == nullptr) {
      printf("malloc threadpool failed\n");
      break;
    }

    pool->min_thr_num = min_thr_num;
    pool->max_thr_num = max_thr_num;
    pool->busy_thr_num = 0;
    pool->live_thr_num = min_thr_num; /* 存活线程数默认为最小值 */
    pool->queue_size = 0;             /* 任务队列默认为0 */
    pool->queue_max_size = queue_max_size;
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->shutdown = false;

    /* 给工作线程开辟空间 */
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_thr_num);
    if (pool->threads == nullptr) {
      printf("malloc thread failed\n");
      break;
    }

    memset(pool->threads, 0, sizeof(pthread_t) * max_thr_num);

    /* 队列开辟空间 */
    pool->task_queue =
        (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_max_size);
    if (pool->task_queue == nullptr) {
      printf("malloc task_queue failed\n");
      break;
    }

    /* 初始化锁 */
    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_mutex_init(&(pool->thread_counter), NULL) != 0 ||
        pthread_cond_init(&(pool->queue_not_empty), NULL) != 0 ||
        pthread_cond_init(&(pool->queue_not_full), NULL) != 0) {
      printf("init the lock or cond failed\n");
      break;
    }

    /* 启动线程池 */
    for (i = 0; i < pool->min_thr_num; ++i) {
      /* 创建每一个线程，并把线程池传入每一个线程 */
      pthread_create(&(pool->threads[i]), NULL, threadpool_thread,
                     (void *)pool);
      printf("start thread 0x%x...\n", (unsigned int)pool->threads[i]);
    }
    /* 创建管理线程 */
    pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void *)pool);
    return pool;
  } while (0);

  threadpool_free(pool);
  return nullptr;
}

void *threadpool_thread(void *threadpool) {
  threadpool_t *pool = (threadpool_t *)threadpool;
  threadpool_task_t task;

  while (true) {
    // 没有任务阻塞等待
    pthread_mutex_lock(&(pool->lock));
    while ((pool->queue_size == 0) && (!pool->shutdown)) {
      printf("thread 0x%x is waiting\n", (unsigned int)pthread_self());
      pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

      /* 自动销毁的部分 */
      /* 如果要退出的线程数目大于0，结束线程*/
      if (pool->wait_exit_thr_num > 0) {
        pool->wait_exit_thr_num--;

        /* 如果线程池内线程大于最小值时可以提前结束当前线程 */
        if (pool->live_thr_num > pool->min_thr_num) {
          printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
          pool->live_thr_num--;
          pthread_mutex_unlock(&(pool->lock));
          pthread_exit(NULL);
        }
      }
    }

    /* 如果指定了true，要关闭线程池，就直接退出，自行退出处理 */
    if (pool->shutdown) {
      pthread_mutex_unlock(&(pool->lock));
      printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
      // 内部唯一一个退出点
      pthread_exit(NULL);
    }

    /* 从任务队列里面获取任务 */
    task.function = pool->task_queue[pool->queue_front].function;
    task.arg = pool->task_queue[pool->queue_front].arg;

    /* 模拟循环队列 */
    pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
    pool->queue_size--;

    /* 通知可以有新的任务添加 */
    pthread_cond_broadcast(&(pool->queue_not_full));

    /* 解锁任务列表· */
    pthread_mutex_unlock(&(pool->lock));

    /* 执行任务 */
    printf("thread 0x%x start working...\n", (unsigned int)pthread_self());
    pthread_mutex_lock(&(pool->thread_counter));
    pool->busy_thr_num++; /* 忙状态线程 */
    pthread_mutex_unlock(&(pool->thread_counter));
    (*(task.function))(task.arg); /* 执行回调函数 */

    /* 任务处理结束 */
    printf("thread 0x%x end working\n", (unsigned int)pthread_self());
    pthread_mutex_lock(&(pool->thread_counter));
    pool->busy_thr_num--;
    pthread_mutex_unlock(&(pool->thread_counter));
  }
  pthread_exit(NULL);
}

void *threadpool_add(threadpool_t *pool, void *(function)(void *arg),
                     void *arg) {
  pthread_mutex_lock(&(pool->lock));

  /* 判断队列是否满 */
  while ((pool->queue_size == pool->queue_max_size) && (!pool->shutdown)) {
    pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
  }

  /* 判断线程池是否关闭 */
  if (pool->shutdown) {
    pthread_mutex_unlock(&(pool->lock));
    printf("thread 0x%x is exiting\n", (unsigned int)pthread_self());
    pthread_exit(NULL);
  }

  /* 清空工作线程的回调函数的参数 */
  if (pool->task_queue[pool->queue_rear].arg != NULL) {
    free(pool->task_queue[pool->queue_rear].arg);
    pool->task_queue[pool->queue_rear].arg = NULL;
  }

  /* 添加任务到任务队列中 */
  pool->task_queue[pool->queue_rear].function = function;
  pool->task_queue[pool->queue_rear].arg = arg;
  pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;
  pool->queue_size++;

  /* 择机唤醒线程池中的任务 */
  pthread_cond_signal(&(pool->queue_not_empty));
  pthread_mutex_unlock(&(pool->lock));

  return nullptr;
}

void *adjust_thread(void *threadpool) {
  int i;
  threadpool_t *pool = (threadpool_t *)threadpool;
  while (!pool->shutdown) {
    sleep(DEFAULT_TIME); /* 定时对线程池管理 */

    /* 关注任务数 和 存活线程数 */
    pthread_mutex_lock(&(pool->lock));
    int queue_size = pool->queue_size;
    int live_thr_num = pool->live_thr_num;
    pthread_mutex_unlock(&(pool->lock));

    pthread_mutex_lock(&(pool->thread_counter));
    int busy_thr_num = pool->busy_thr_num;
    pthread_mutex_unlock(&(pool->thread_counter));

    /* 如果 任务数大于最小线程数 并且 存活线程数小于最大线程个数 */
    if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num) {
      pthread_mutex_lock(&(pool->lock));
      int add = 0;

      /* 一次增加 DEFAULT_THREAD 个线程 */
      for (i = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY &&
                  pool->live_thr_num < pool->max_thr_num;
           i++) {
        if (pool->threads[i] == 0 || !is_thread_alive(pool->threads[i])) {
          pthread_create(&(pool->threads[i]), NULL, threadpool_thread,
                         (void *)pool);
          add++;
        }
      }
      pthread_mutex_unlock(&(pool->lock));
    }

    /* 销毁多余线程 忙线程×2 小于 存活线程数 且 存活线程数大于最小线程数 */
    if ((busy_thr_num * 2) < live_thr_num && live_thr_num > pool->min_thr_num) {
      /* 一次销毁 DEFAULT_THREAD 线程, 随机选取 */
      pthread_mutex_lock(&(pool->lock));
      /* 要销毁的线程数 */
      pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
      pthread_mutex_unlock(&(pool->lock));

      for (i = 0; i < DEFAULT_THREAD_VARY; ++i) {
        /* 通知处在空闲状态的线程, 他们胡自行销毁*/
        pthread_cond_signal(&(pool->queue_not_empty));
      }
    }
  }
  return NULL;
}

int threadpool_destroy(threadpool_t *pool) {
  int i;
  if (pool == nullptr) {
    return -1;
  }

  pool->shutdown = true;

  /* 先销毁管理者线程 */
  pthread_join(pool->adjust_tid, NULL);

  /* 通知所有空闲线程 */
  pthread_cond_broadcast(&(pool->queue_not_empty));

  for (i = 0; i < pool->live_thr_num; ++i) {
    pthread_join(pool->threads[i], NULL);
  }

  threadpool_free(pool);
  return 0;
}

int threadpool_free(threadpool_t *pool) {
  if (pool == nullptr) return -1;

  if (pool->task_queue) free(pool->task_queue);

  if (pool->threads) {
    free(pool->threads);
    pthread_mutex_lock(&(pool->lock));
    pthread_mutex_destroy(&(pool->lock));
    pthread_mutex_lock(&(pool->thread_counter));
    pthread_mutex_destroy(&(pool->thread_counter));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
  }
  free(pool);
  pool = NULL;
  return 0;
}

int threadpool_all_threadnum(threadpool_t *pool) {
  int all_threadnum = -1;
  pthread_mutex_lock(&(pool->lock));
  all_threadnum = pool->live_thr_num;
  pthread_mutex_unlock(&(pool->lock));
  return all_threadnum;
}

int threadpool_busy_threadnum(threadpool_t *pool) {
  int busy_threadnum = -1;
  pthread_mutex_lock(&(pool->thread_counter));
  busy_threadnum = pool->busy_thr_num;
  pthread_mutex_unlock(&(pool->thread_counter));
  return busy_threadnum;
}

bool is_thread_alive(pthread_t tid) {
  /* 0号信号判断是否存活 */
  int kill_rc = pthread_kill(tid, 0);
  if (kill_rc == ESRCH) {
    return false;
  }
  return true;
}