/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-07 09:26:14
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-07 09:55:13
 * @Description: example of mmap
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  size_t copy = 4;
  char* p = nullptr;
  int fd = open("mytext.txt", O_CREAT | O_RDWR, 0644);
  if (fd < 0) {
    perror("open()");
    exit(1);
  }

  int e = ftruncate(fd, 4);
  if (e == -1) {
    perror("ftruncate()");
    exit(1);
  }

  p = (char*)mmap(nullptr, copy, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED) {
    perror("mmap()");
    exit(1);
  }

  close(fd);
  strcpy(p, "abc");

  int ret = munmap(p, copy);
  if (ret == -1) {
    perror("munmap()");
    exit(1);
  }

  return 0;
}
