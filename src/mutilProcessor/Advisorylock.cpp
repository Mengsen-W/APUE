/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-10 16:34:25
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-10 17:28:25
 * @Description: 文件锁借助fcntl 达成对文件的读写锁
 * 不能再线程中使用，因为公用一个文件描述符导致公用一个flock结构体
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sys_err(char* str) {
  perror(str);
  exit(1);
}

int main(int argc, char** argv) {
  int fd;
  struct flock f_lock;

  if (argc < 2) {
    printf("Usage: ...");
    exit(1);
  }

  if ((fd = open(argv[1], O_RDWR)) < 0) {
    sys_err("open()");
  }

  f_lock.l_type = F_WRLCK;
  // f_lock.l_type = F_RDLCK;  /*读锁*/

  f_lock.l_whence = SEEK_SET;
  f_lock.l_start = 0;
  f_lock.l_len = 0; /* 整个文件加锁 */

  fcntl(fd, F_SETLKW, &f_lock);
  printf("get flock\n");

  sleep(10);

  f_lock.l_type = F_UNLCK;
  fcntl(fd, F_SETLKW, &f_lock);
  printf("unlock");

  close(fd);

  return 0;
}
