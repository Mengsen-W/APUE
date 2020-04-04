/*
  * @Author: Mengsen.Wang
  * @Date:
  * @Last Modified by: Mengsen.Wang
  * @Last Modified time: 
  * @Description: 
*/
#include <iostream>
extern char** environ;
int main() {
  int i = 0;
  for(i = 0; environ[i]; ++i) {
    std::cout << environ[i] << std::endl;
  }
  return 0;
}
