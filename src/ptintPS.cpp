/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-03 15:34:47
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-03 15:51:42
 * @Description: 用dup2和exce 打印环境信息
 */

//* dup2(a, b) b指向a

#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
int main(void) {
  int fd;
  fd = open("ps.out", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    std::cerr << "open ps.out error" << std::endl;
    exit(1);
  }

  dup2(fd, STDOUT_FILENO);

  execlp("ps", "ps", "ax", NULL);

  // 出错才返回，不出错不会返回值，而是返回到了调用的函数里面
  // 若成功其实已经改变了分支
  // 这里返回的是是否成功置换过
  std::cerr << "error execlp" << std::endl;
  close(fd);
  return 0;
}