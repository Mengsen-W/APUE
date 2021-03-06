/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-05 22:31:01
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-05 22:54:14
 * @Description: 有限状态机 select 版本
 */
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"
#define BUFSIZE 1024

enum { STATE_R, STATE_W, STATE_Ex, STATE_T };

struct fsm_st {
  int status;
  int sfd;
  int dfd;
  int len;
  int pos;
  char buf[BUFSIZE];
  char* error;
};

static void fsm_driver(struct fsm_st* fsm) {
  int ret = 0;
  switch (fsm->status) {
    case STATE_R:
      fsm->len = read(fsm->sfd, fsm->buf, BUFSIZ);
      if (fsm->len == 0)
        fsm->status = STATE_T;
      else if (fsm->len < 0) {
        if (errno == EAGAIN)
          fsm->status = STATE_R;
        else {
          fsm->error = "read()";
          fsm->status = STATE_Ex;
        }
      } else {
        fsm->pos = 0;
        fsm->status = STATE_W;
      }
      break;
    case STATE_W:
      ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
      if (ret < 0) {
        if (errno == EAGAIN)
          fsm->status = STATE_W;
        else {
          fsm->error = "write()";
          fsm->status = STATE_Ex;
        }
      } else {
        fsm->pos += ret;
        fsm->len -= ret;
        if (fsm->len == 0)
          fsm->status = STATE_R;
        else
          fsm->status = STATE_W;
      }
      break;
    case STATE_Ex:
      perror("fsm->error");
      fsm->status = STATE_T;
      break;
    case STATE_T:
      /* do something*/
      break;
    default:
      abort();
      break;
  }
}

void relay(int fd1, int fd2) {
  struct fsm_st fsm12, fsm21;

  int fd1_save = fcntl(fd1, F_GETFL);
  fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);
  int fd2_save = fcntl(fd2, F_GETFL);
  fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);

  fsm12.status = STATE_R;
  fsm12.sfd = fd1;
  fsm12.dfd = fd2;

  fsm21.status = STATE_R;
  fsm21.sfd = fd2;
  fsm21.dfd = fd1;

  while (fsm12.status != STATE_T || fsm21.status != STATE_T) {
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    if (fsm12.status == STATE_R) FD_SET(fsm12.sfd, &rset);
    if (fsm12.status == STATE_W) FD_SET(fsm12.dfd, &wset);
    if (fsm21.status == STATE_R) FD_SET(fsm21.sfd, &rset);
    if (fsm21.status == STATE_W) FD_SET(fsm21.dfd, &wset);

    if (select(std::max(fd1, fd2) + 1, &rset, &wset, NULL, NULL) < 0) {
      if (errno == EINTR) continue;
      perror("select()");
      exit(1);
    }
    if (FD_ISSET(fd1, &rset) || FD_ISSET(fd2, &wset)) fsm_driver(&fsm12);
    if (FD_ISSET(fd2, &rset) || FD_ISSET(fd1, &wset)) fsm_driver(&fsm12);
  }

  fcntl(fd1, F_SETFL, fd1_save);
  fcntl(fd2, F_SETFL, fd2_save);
}

int main() {
  int fd1 = open(TTY1, O_RDWR);
  if (fd1 < 0) {
    perror("open()");
    exit(1);
  }
  write(fd1, "TTY1\n", 5);

  int fd2 = open(TTY2, O_RDWR | O_NONBLOCK);
  if (fd2 < 0) {
    perror("open()");
    exit(1);
  }
  write(fd2, "TTY2\n", 5);

  relay(fd1, fd2);
  close(fd1);
  close(fd2);
  return 0;
}
