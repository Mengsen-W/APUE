/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 11:32:06
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 21:46:43
 * @Description: 守护进程
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#define FNAME "/tmp/out"

static int daemonize() {
  int fd;
  pid_t pid;
  pid = fork();
  if (pid < 0) return -1;

  // 此处父进程退出
  if (pid > 0) exit(0);

  fd = open("/dev/null", O_RDWR);
  if (fd < 0) return -1;
  dup2(fd, 0);
  dup2(fd, 1);
  dup2(fd, 2);

  if (fd > 2) close(fd);

  setsid();
  //保证稳定性
  if (chdir("/")) return -1;
  return 0;
}

int main() {
  FILE* fp;
  openlog("mydaemonize", LOG_PID, LOG_DAEMON);
  if (daemonize()) {
    syslog(LOG_ERR, "daemonize() failed");
    exit(1);
  } else {
    syslog(LOG_INFO, "daemonize() succeeded");
  }
  fp = fopen(FNAME, "w");

  if (fp == NULL) {
    syslog(LOG_ERR, "fopen() failed : %s", strerror(errno));
    exit(1);
  }
  for (int i = 0;; ++i) {
    fprintf(fp, "%d\n", i);
    fflush(fp);
    sleep(1);
  }
  fclose(fp);
  closelog();
  return 0;
}