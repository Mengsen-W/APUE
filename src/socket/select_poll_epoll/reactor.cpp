/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-13 16:44:23
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-13 16:58:15
 * @Description:  reactor
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <cstring>

#define MAX_EVENTS 1024
#define BUFLEN 4096
#define SERV_PORT 6666

/* 初始化监听套接字 */
void init_listen_socket(int fd, int port);

/* 增加事件 */
void eventadd(int efd, int events, struct myevent_s *ev);
/* 删除事件 */
void eventdel(int fd, struct myevent_s *ev);
/* 初始化事件 */
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *),
              void *arg);

/* 封装连接 */
void acceptconn(int lfd, int events, void *arg);

/* 接受数据 */
void recvdata(int fd, int events, void *arg);
/* 发送数据 */
void senddata(int fd, int events, void *arg);

/* 描述就绪文件描述符相关信息 */
struct myevent_s {
  int fd;      // 要监听的文件描述符
  int events;  // 对应的监听事件
  void *arg;   // 泛型参数
  void (*call_back)(int fd, int events, void *arg);  // 回调函数
  int status;        // 是否在监听 1在监听 0不在监听
  char buf[BUFLEN];  // buffer
  int len;           // buffer length
  long last_active;  // 记录每次加入红黑树 g_efd 的时间值，若超时没动作断开连接
};

int g_efd;  // 保存epoll_creat返回的文件描述符
struct myevent_s g_events[MAX_EVENTS + 1];  // 自定义结构体数组 +1 listenfd

/* main 函数 */
int main(int argc, char *argv[]) {
  unsigned short port = SERV_PORT;

  if (argc == 2) {
    port = atoi(argv[1]); /*可以由用户指定端口*/
  }

  g_efd = epoll_create(MAX_EVENTS + 1); /* 初始化事件板 */
  if (g_efd <= 0) {
    printf("creat efd in %s err %s \n", __func__, strerror(errno));
  }

  init_listen_socket(g_efd, port);
  struct epoll_event events[MAX_EVENTS + 1];
  printf("server running: port [%d]\n", port);

  int checkpos = 0, i = 0;
  while (1) {
    long now = time(NULL);
    for (i = 0; i < 100; ++i, ++checkpos) { /* 一次循环检测100个 */
      if (checkpos == MAX_EVENTS) checkpos = 0;
      if (g_events[checkpos].status != 1) continue; /* 不在树上 */

      long duration = now - g_events[checkpos].last_active; /* 不活跃的时间 */

      if (duration >= 60) {
        close(g_events[checkpos].fd);
        printf("[fd = %d] timeout\n", g_events[checkpos].fd);
        eventdel(g_efd, &g_events[checkpos]);
      }
    }

    /* 监听，将满足事件的文件描述父加入events数组中，1秒没有事件满足就返回0 */
    int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
    if (nfd < 0) {
      printf("epoll_wait error, exit\n");
      break;
    }

    for (i = 0; i < nfd; ++i) {
      struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;

      if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
        ev->call_back(ev->fd, events[i].events, ev->arg);
      if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
        ev->call_back(ev->fd, events[i].events, ev->arg);
    }
  }
  return 0;
}

/* 初始化监听套接字 */
void init_listen_socket(int fd, int port) {
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  /* listen 套接字设置阻塞 */
  fcntl(lfd, F_SETFL, O_NONBLOCK);

  /* 把 listenfd 放在数组最后一个位置，初始化结构体 */
  eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
  /* 加入事件板 */
  eventadd(g_efd, EPOLLIN, &g_events[MAX_EVENTS]);

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = INADDR_ANY;

  bind(lfd, (struct sockaddr *)&sin, sizeof(sin));
  listen(lfd, SOMAXCONN);

  return;
}

/* 将结构体 myevent_s 成员变量初始化 */
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *),
              void *arg) {
  ev->fd = fd;
  ev->events = 0;
  ev->arg = arg;
  ev->call_back = call_back;
  ev->status = 0;
  memset(ev->buf, 0, sizeof(ev->buf));
  ev->len = 0;
  ev->last_active = time(NULL);  // 调用 eventset 的时间
  return;
}

/* 添加套接字到事件板 */
void eventadd(int efd, int events, struct myevent_s *ev) {
  struct epoll_event epv = {0, {0}};
  int op;
  epv.data.ptr = ev;
  epv.events = ev->events = events;

  if (ev->status == 1) {
    op = EPOLL_CTL_MOD;
  } else {
    op = EPOLL_CTL_ADD;
    ev->status = 1;
  }

  if (epoll_ctl(g_efd, op, ev->fd, &epv) < 0)
    printf("event add failed [fd = %d] [events = %d]\n", ev->fd, events);
  else
    printf("event add success [fd = %d] [op = %d] [events = %d]\n", ev->fd, op,
           events);
  return;
}

/* 删除事件 */
void eventdel(int epfd, struct myevent_s *ev) {
  struct epoll_event epv = {0, {0}};

  if (ev->status != 1) return; /* 不再红黑树上 */

  epv.data.ptr = ev;
  ev->status = 0;
  epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &epv);

  return;
}

/* 建立连接 accept 的封装 */
void acceptconn(int lfd, int events, void *arg) {
  struct sockaddr_in cin;
  socklen_t len = sizeof(cin);
  int cfd, i;

  if ((cfd = accept(lfd, (sockaddr *)&cin, &len)) == -1) {
    if (errno != EAGAIN && errno != EINTR) {
      /* 出错处理先不写了，最好是用回调函数回调出去 */
    }
    /* 报错退出 */
    printf("%s: accept, %s\n", __func__, strerror(errno));
    return;
  }

  do {                               /* 用 do while 达到 goto 效果 */
    for (i = 0; i < MAX_EVENTS; ++i) /* 放入连接套接字 */
      if (g_events[i].status == 0)   /* 找到位置 */
        break;
    if (i == MAX_EVENTS) { /* 没找到位置 */
      printf("%s: max connect limit [%d]\n", __func__, MAX_EVENTS);
      break;
    }
    int flag = 0;
    if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) { /* 设置非阻塞 */
      printf("%s: fcntl nonblock failed, %s\n", __func__, strerror(errno));
      break;
    }

    /* 初始化cfd并加入事件板 */
    eventset(&g_events[i], cfd, recvdata, &g_events[i]);
    eventadd(g_efd, EPOLLIN, &g_events[i]);
  } while (0);

  printf("new connect [%s : %d] [time = %ld], [pos = %d]",
         inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active,
         i);

  return;
}

/* 接受数据 */
void recvdata(int fd, int events, void *arg) {
  struct myevent_s *ev = (struct myevent_s *)arg;
  int len;

  len = recv(fd, ev->buf, sizeof(ev->buf), 0);

  eventdel(g_efd, ev); /* 删除节点 */

  if (len > 0) { /* 确实有数据可读 */
    ev->len = len;
    ev->buf[len] = '\0';
    printf("recv [%d] : %s\n", fd, ev->buf);

    eventset(ev, fd, senddata, ev);
    eventadd(g_efd, EPOLLOUT, ev);
  } else if (len == 0) { /* 对方关闭 */
    close(ev->fd);
    /* 地址相减得到下标 */
    printf("[fd = %d] [pos = %ld] closed\n", fd, ev - g_events);
  } else { /* 出现错误 */
    close(ev->fd);
    printf("recv [fd = %d] [ error = %d %s]\n", fd, errno, strerror(errno));
  }

  return;
}

/* 发送数据 */
void senddata(int fd, int events, void *arg) {
  struct myevent_s *ev = (struct myevent_s *)arg;
  int len;

  len = send(fd, ev->buf, ev->len, 0);

  if (len > 0) { /* 成功 */
    printf("send [fd = %d] [len = %d] [buf = %s]\n", fd, len, ev->buf);
    eventdel(g_efd, ev);
    eventset(ev, fd, recvdata, ev);
    eventadd(g_efd, EPOLLOUT, ev);
  } else { /* 出错 */
    printf("send [fd = %d] error %s\n", fd, strerror(errno));
    close(fd);
    eventdel(g_efd, ev);
  }

  return;
}
