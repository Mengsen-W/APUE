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
  //!! 以读写的方式打开文件,若存在即报错,若不存在即报错
  //!! 打开文件和创建文件之间是原子操作
  //!! Open 可以很理想的代替 creat
  //!! int fd = open("test_1.txt", O_RDWR | O_CREAT | O_EXCL, 0666);

  //!! 如果创建文件失败则打印错误并且不向文件内写入流
  //!! 不加 O_EXCL 则不会检查文件是否存在
  // if (fd == -1) {
  // std::cout << "Creat File Error" << std::endl;
  // return -1;
  // }
  // write(fd, "Hello", 5);

  int fd = open("test_1.txt", O_RDWR);
  if (fd == -1) {
    std::cout << "Open File Error" << std::endl;
    return -1;
  }

  //!! 可以利用 lseek 设置偏移量
  // off_t currpos;
  //!! 返回文件当前偏移量,就是最后插入的偏移量
  // currpos = lseek(fd, 0, SEEK_CUR);
  // std::cout << currpos << std::endl;
  //!! 默认偏移量是 SEEK_SET 既是0
  //!! 是把字符串写到文件偏移量的位置,最后一个参数制定写入的字符长度
  //!! write 返回值与最后1个参数不同时,表示出错
  // write(fd, "!", 1);
  // currpos = lseek(fd, 0, SEEK_CUR);
  // std::cout << currpos << std::endl;
  // currpos = lseek(fd, 0, SEEK_END);
  // write(fd, "!", 1);
  // currpos = lseek(fd, 0, SEEK_CUR);
  // std::cout << currpos << std::endl;
  // currpos = lseek(fd, 3, SEEK_SET);
  // write(fd, "!", 1);
  // currpos = lseek(fd, 0, SEEK_CUR);
  // std::cout << currpos << std::endl;
  //!! 写完后文件偏移量会移动到下一个位置
  //!! 如返回 -1 则可以确定该文件不支持文件偏移量

  // !!能读出多少和文件字符手有关,若读太大则只返回文件字符数
  // ssize_t read_nums;
  // char buff[20] = {0};
  // read_nums = read(fd, buff, 20);
  // std::cout << read_nums << std::endl;
  // std::cout << buff << std::endl;
  return 0;
}
