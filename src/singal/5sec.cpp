/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-04 16:21:05
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-04 16:24:38
 * @Description: 定时5秒累加多次
 */
#include <time.h>

#include <cstdio>
#include <cstdlib>

int main() {
  time_t end;
  long long int count = 0;
  end = time(NULL) + 5;
  while (time(NULL) <= end) {
    ++count;
  }
  printf("%lld\n", count);
  return 0;
}