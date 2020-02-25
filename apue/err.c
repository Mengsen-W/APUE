/*
 * @Author: Mengsen.Wang
 * @Date: 2020-02-24 20:21:30
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-02-24 20:25:29
 */

#include "apue.h"
#include <errno.h>  /* for definition of errno */
#include <stdarg.h> /* ISO C variable aruments */
static void err_doit(int, int, const char *, va_list);
/*
   err_ret非致命错误输出
   与err_sys相比，没有exit()，并不退出函数。
   相同的是都调用stderror(errno)打印详细出错信息。
   可变参数列表中多为vsnprintf等打印参数的参数列表。
 */
void err_ret(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void err_sys(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  exit(1); /* 退出 */
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 * 不调用系统打印出错信息。
 */
void err_exit(int error, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, error, fmt, ap);
  va_end(ap);
  exit(1);
}

/*
 * 不调用系统出错信息，但退出程序
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void err_dump(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  abort(); /* dump core and terminate */
  exit(1); /* shouldn't get here */
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void err_msg(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
}

/*
 * 不调用系统出错信息，但退出程序
 * Print a message and terminate.
 */
void err_quit(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
  exit(1);
}

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap) {
  char buf[MAXLINE];

  vsnprintf(buf, MAXLINE, fmt,
            ap); /* vsnprintf与snprintf区别在于vsnprintf调用可变参数列表 */
  if (errnoflag) /* 在buf后追加系统出错信息 */
    snprintf(buf + strlen(buf), MAXLINE - strlen(buf), ": %s", strerror(error));
  strcat(buf, "\n");
  fflush(stdout); /* in case stdout and stderr are the same */
  fputs(buf, stderr);
  fflush(NULL); /* flushes all stdio output streams */
}