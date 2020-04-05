/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-31 11:15:41
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-31 11:31:29
 * @Description: 循环读写
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main() {
  std::cout << "Hello world" << std::endl;
  // 打开一个已经存在的文件
  int fd = open("open.txt", O_RDONLY);
  if (fd == 1) {
    perror("open");
    exit(1);
  }
  // 创建一个新文件
  int fd1 = open("newfile", O_CREAT | O_WRONLY, 0664);
  if (fd1 == 1) {
    perror("open");
    exit(1);
  }

  // 读数据
  char buf[2048] = {0};
  int count = read(fd, buf, sizeof(buf));

  if (count == -1) {
    perror("read");
    exit(1);
  }

  while (count) {
    int ret = write(fd1, buf, count);
    printf("write bytes %d\n", ret);
    count = read(fd, buf, sizeof(buf));
  }

  close(fd);
  close(fd1);
  return 0;
}
