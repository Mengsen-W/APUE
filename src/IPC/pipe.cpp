/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-03 17:21:11
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-06 16:15:08
 * @Description: 测试管道通信
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main() {
  int fd[2];
  pid_t pid;
  int ret = pipe(fd);
  if (ret == -1) {
    std::cerr << "pipe() failed" << std::endl;
    exit(1);
  }
  pid = fork();
  if (pid == -1) {
    std::cerr << "fork() failed" << std::endl;
    exit(1);
  } else if (pid == 0) {  //读数据
    close(fd[1]);
    char buf[1024];
    int ret = read(fd[0], buf, sizeof(buf));
    if (ret == 0) {
      std::cout << "read end" << std::endl;
    }
    if (write(STDOUT_FILENO, buf, ret)) {
      close(fd[0]);
      return 0;
    }
  } else {  //父进程
    close(fd[0]);
    if (write(fd[1], "hello pipe\n", sizeof("hello pipe\n"))) {
      close(fd[1]);
      wait(nullptr);
      return 0;
    }
  }
  return 0;
}