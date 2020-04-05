/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-31 18:37:02
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-31 19:53:01
 * @Description: 实现 ls -l 命令
 */

#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: [filename]..." << std::endl;
    exit(1);
  }

  struct stat st;
  int ret = stat(argv[1], &st);
  if (ret == -1) {
    perror("stat");
    exit(-1);
  }

  // 存储文件类型和访问权限
  char perms[11] = {0};
  // 判断未见类型
  switch (st.st_mode & S_IFMT) {
    case S_IFLNK:
      perms[0] = '1';
      break;
    case S_IFDIR:
      perms[0] = 'd';
      break;
    case S_IFREG:
      perms[0] = '-';
      break;
    case S_IFBLK:
      perms[0] = 'b';
      break;
    case S_IFCHR:
      perms[0] = 'c';
      break;
    case S_IFSOCK:
      perms[0] = 's';
      break;
    case S_IFIFO:
      perms[0] = 'p';
      break;
    default:
      perms[0] = '?';
      break;
  }

  // 判断文件的访问权限
  // 文件所有者
  perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
  perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
  perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
  // 文件所属组
  perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
  perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
  perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
  // 其他人
  perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
  perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
  perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

  // 硬链接数目
  int linkNum = st.st_nlink;
  // 文件所有者
  char* fileUser = getpwuid(st.st_uid)->pw_name;
  // 文件所属组
  char* fileGrp = getgrgid(st.st_gid)->gr_name;
  // 文件大小
  int fileSize = (int)st.st_size;
  // 修改时间
  char* time = ctime(&st.st_mtime);
  char mtine[512] = {0};
  strncpy(mtine, time, strlen(time) - 1);

  char buf[1024];
  sprintf(buf, "%s %d %s %s %d %s %s", perms, linkNum, fileUser, fileGrp,
          fileSize, mtine, argv[1]);
  printf("%s\n", buf);
  return 0;
}