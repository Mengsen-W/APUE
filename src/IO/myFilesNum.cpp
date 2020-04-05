/*
 * @Author: Mengsen.Wang
 * @Date: 2020年4月1日 16点20分
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time:
 * @Description: 读取目录下文件的数目
 */

#include <dirent.h>
#include <string.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

int getNumber(const char* root) {
  DIR* dir = nullptr;
  dir = opendir(root);
  if (dir == nullptr) {
    perror("opendir error");
    exit(1);
  }

  char path[1024] = {0};
  int total = 0;
  struct dirent* ptr = nullptr;
  while ((ptr = readdir(dir)) != nullptr) {
    if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
      continue;
    }
    if (ptr->d_type == DT_DIR) {
      sprintf(path, "%s%s", root, ptr->d_name);
      total += getNumber(path);
    }
    if (ptr->d_type == DT_REG) {
      ++total;
    }
  }
  closedir(dir);
  return total;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: path name" << std::endl;
    exit(1);
  }

  int total = 0;
  total = getNumber(argv[1]);
  std::cout << "the number of path name = " << total << std::endl;
  return 0;
}
