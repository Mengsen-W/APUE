/*
 * @Author: Mengsen.Wang
 * @Date: 2020-03-13 09:34:07
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-03-13 09:56:16
 */

#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std;
void myPrint(int num) {
  std::cout << "Begin Thread = " << num << std::endl;
  std::cout << "End Thread = " << num << std::endl;
}

int main() {
  std::vector<std::thread> mythread;
  for (int i = 0; i < 10; ++i) {
    mythread.push_back(std::thread(myPrint, i));
  }
  for (auto iter = mythread.begin(); iter != mythread.end(); ++iter) {
    iter->join();
  }

  std::cout << "End of main()" << std::endl;

  return 0;
}