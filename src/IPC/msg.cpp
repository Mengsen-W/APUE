/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-06 17:14:54
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-06 19:57:06
 * @Description: message queue 测试
 */

// 如果发送方退出接收方会产生未知的 LeakSanitizer has encountered a fatal error.
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define KEYPATH "/etc/services"
#define KEYPROJ 'a'
#define NAMESIZE 8

struct msg_st {
  char name[NAMESIZE];
  int math;
  int chinese;
  long mtype;
};

void *sender(void *) {
  struct msg_st sbuf;
  key_t key = ftok(KEYPATH, KEYPROJ);
  if (key < 0) {
    perror("ftok()");
    exit(-1);
  }

  int msgid = msgget(key, 0);
  if (msgid < 0) {
    perror("msgget()");
    exit(-1);
  }
  while (1) {
    sbuf.mtype = 1;
    strcpy(sbuf.name, "Wms");
    sbuf.math = rand() % 100;
    sbuf.chinese = rand() % 100;
    if (msgsnd(msgid, &sbuf, sizeof(struct msg_st) - sizeof(long), 0) < 0) {
      perror("msgrcv()");
      exit(1);
    }
  }
  puts("ok");

  exit(0);
}

void *rcver(void *) {
  struct msg_st rbuf;

  key_t key = ftok(KEYPATH, KEYPROJ);
  if (key < 0) {
    perror("ftok()");
    exit(-1);
  }

  int msgid = msgget(key, IPC_CREAT | 0600);
  if (msgid < 0) {
    perror("msgget()");
    exit(-1);
  }

  while (true) {
    if (msgrcv(msgid, &rbuf, sizeof(struct msg_st) - sizeof(long), 0, 0) < 0) {
      perror("msgrcv()");
      exit(1);
    }
    printf("Name = %s\n", rbuf.name);
    printf("Math= %d\n", rbuf.math);
    printf("Chinese = %d\n", rbuf.chinese);
  }

  if (msgctl(msgid, IPC_RMID, nullptr) != 0) {
    perror("msgeixt()");
    exit(1);
  }

  exit(0);
}

int main() {
  pthread_t tid1, tid2;
  puts("Begin");

  int err1 = pthread_create(&tid1, NULL, &rcver, NULL);
  if (err1) {
    perror("creat() t1 error");
    exit(1);
  }

  // sleep(3);

  int err2 = pthread_create(&tid2, NULL, &sender, NULL);
  if (err2) {
    perror("creat() t0 error");
    exit(1);
  }
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  puts("End");

  return 0;
}
