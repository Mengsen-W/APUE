/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-02 15:11:28
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-02 15:38:56
 * @Description: 循环创建子进程
 */

#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main() {
  pid_t pid = getpid();
  std::cout << "----- Start ----- this pid = " << pid << std::endl;
  for (int i = 0; i < 5; ++i) {
    pid = fork();
    if (pid == -1) {
      perror("fork error");
      exit(1);
    } else if (pid == 0) {
      //* 子进程出口，防止子进程继续执行fork() 循环
      std::cout << "son pid = " << getpid() << " father pid = " << getppid()
                << std::endl;
      break;
    }
    if (pid != 0) {
      std::cout << "father pid = " << getpid()
                << " father of father pid = " << getppid() << std::endl;
      // 防止子进程成为孤儿进程
      sleep(1);
    }
  }
  return 0;
}