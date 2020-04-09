/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-09 09:27:33
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-09 09:35:44
 * @Description: 守护进程
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  pid_t pid, sid;
  pid = fork();
  if (pid > 0) return 0;

  sid = setsid();

  int ret = chdir("/home/");
  if (ret < 0) {
    perror("chdir()");
    exit(1);
  }

  umask(0002);

  close(0);
  open("/dev/null", O_RDWR);
  dup2(0, STDOUT_FILENO);
  dup2(0, STDERR_FILENO);

  // 主逻辑
  while (1) {
    sleep(1);
  }

  return 0;
}
