/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-06 12:45:40
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-06 12:59:21
 * @Description: mmap 例子
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage:....\n");
    exit(1);
  }

  int fd = open(argv[1], O_RDONLY);
  struct stat statres;
  if (fd < 0) {
    perror("open()");
    exit(1);
  }
  if (fstat(fd, &statres)) {
    perror("fstat()");
    exit(1);
  }
  void* temp = mmap(nullptr, statres.st_size, PROT_READ, MAP_SHARED, fd, 0);
  char* str = (char*)temp;
  if (str == MAP_FAILED) {
    perror("mmap()");
    exit(1);
  }

  close(fd);
  int count = 0;
  for (int i = 0; i < statres.st_size; ++i) {
    if (str[i] == 'c') ++count;
  }
  printf("%d\n", count);

  munmap(temp, statres.st_size);
  return 0;
}