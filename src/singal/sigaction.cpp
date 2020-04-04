/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 11:32:06
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 22:01:03
 * @Description: 测试 sigaction()
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#define FNAME "/tmp/out"

static FILE* fp;
static int daemonize() {
  int fd;
  pid_t pid;
  pid = fork();
  if (pid < 0) return -1;
  if (pid > 0) exit(1);
  fd = open("/dev/null", O_RDWR);
  if (fd < 0) return -1;
  dup2(fd, 0);
  dup2(fd, 1);
  dup2(fd, 2);

  if (fd > 2) close(fd);

  setsid();
  if (chdir("/")) return -1;
  return 0;
}

static void daemonize_exit(int s) {
  fclose(fp);
  closelog();
}

int main() {
  openlog("mydaemonize", LOG_PID, LOG_DAEMON);

  struct sigaction sa;
  sa.sa_handler = daemonize_exit;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGQUIT | SIGTERM | SIGINT);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  // signal(SIGINT, daemonize_exit);
  // signal(SIGQUIT, daemonize_exit);
  // signal(SIGTERM, daemonize_exit);
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
  return 0;
}