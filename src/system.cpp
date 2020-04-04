/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 10:59:48
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 11:05:57
 * @Description: 测试system()
 */

#include <cstdlib>
#include <iostream>

int main() {
  if (system("date +%s > /tmp/out")) exit(1);
  // 等价于 exel("/bin/sh", "sh", "-c", "date +%s",">", "/tmp/out",NULL);
  return 0;
}