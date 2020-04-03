/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-03 16:01:45
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-03 16:21:30
 * @Description: 使用 wait / waitid 回收 zombie
 */

#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main() {
  int status;
  pid_t pid, wpid;
  pid = fork();
  if (pid == 0) {
    std::cout << "----child, my parent = " << getpid() << std::endl;
    sleep(30);
    std::cout << "----child die----" << std::endl;

  } else if (pid > 0) {
    // z阻塞等待子进程，子进程结束后回收子进程
    wpid = wait(&status);

    if (wpid == -1) {
      std::cerr << "error wait" << std::endl;
      exit(1);
    }

    // 正常结束
    if (WIFEXITED(status)) {
      std::cout << "收尸成功，自己死亡" << WEXITSTATUS(status) << std::endl;
    }

    //信号终止错误
    if (WIFSIGNALED(status)) {
      std::cout << "收尸成功，信号收尸" << WTERMSIG(status) << std::endl;
    }

    while (1) {
      std::cout << "----Parent ,pid = " << getpid() << "my son = " << pid
                << std::endl;
      sleep(1);
    }
  } else
    std::cerr << "error fork" << std::endl;
  return 0;
}