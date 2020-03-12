/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-12 11:36:44
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-12 11:54:11
 */

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXSIZE 4096

int mydup2(int fd1, int fd2) {
  if (fd1 < 0 || fd2 < 0) return -1;
  if (fd1 == fd2) return fd1;

  close(fd2);

  int fd_arr[MAXSIZE] = {0}, num = 0, fd_tmp = 0;
  while ((fd_tmp = dup(fd1)) < fd2) {
    fd_arr[num] = fd_tmp;
    ++num;
  }
  while (num > 0) {
    close(fd_arr[num]);
    --num;
  }
  return fd2;
}

int main() {
  /*
   * 实现 dup2 的功能
   */

  return 0;
}
