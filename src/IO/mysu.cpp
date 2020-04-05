/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 10:41:12
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 10:48:53
 * @Description: 获取其他用户权限
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
  pid_t pid;
  if (argc < 3) {
    std::cerr << "Usage..." << std::endl;
    exit(1);
  }
  pid = fork();
  if (pid < 0) {
    std::cerr << "Failed to fork" << std::endl;
  }
  if (pid == 0) {
    if (setuid(atoi(argv[1]))) {
      std::cerr << "Failed to setuid" << std::endl;
    }
    execvp(argv[2], argv + 2);
    std::cerr << "Failed to exec" << std::endl;
    exit(1);
  }

  wait(NULL);
  return 0;
}
