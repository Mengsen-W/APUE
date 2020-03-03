/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-03 21:22:41
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-03 21:54:55
 */

/*
 * 文件IO一共 open read write lseek close 五个函数
 * 这五个函数全部不带缓存
 */

/*
$ ulimit -a
-t: cpu time (seconds)              unlimited
-f: file size (blocks)              unlimited
-d: data seg size (kbytes)          unlimited
-s: stack size (kbytes)             8192
-c: core file size (blocks)         0
-m: resident set size (kbytes)      unlimited
-u: processes                       15275
-n: file descriptors                8192
-l: locked-in-memory size (kbytes)  65536
-v: address space (kbytes)          unlimited
-x: file locks                      unlimited
-i: pending signals                 15275
-q: bytes in POSIX msg queues       819200
-e: max nice                        0
-r: max rt priority                 0
-N 15:                              unlimited
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char *argv[]) {
  // 以读写的方式打开文件,若存在即报错,若不存在即报错
  //打开文件和创建文件之间是原子操作
  int fd = open("test_1.txt", O_RDWR | O_CREAT | O_EXCL, 0666);

  // 如果创建文件失败则打印错误并且不向文件内写入流
  // 不加 O_EXCL 则不会检查文件是否存在
  if (fd == -1) {
    std::cout << "Creat File Error" << std::endl;
    return -1;
  }

  write(fd, "Hello World!", 12);
  return 0;
}
