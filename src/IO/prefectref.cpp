/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-03 07:42:20
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-03 07:43:28
 */
#include <iostream>
using namespace std;

void fun(int& x) { cout << "call lvalue ref" << endl; }
void fun(int&& x) { cout << "call rvalue ref" << endl; }
void fun(const int& x) { cout << "call const lvalue ref" << endl; }
void fun(const int&& x) { cout << "call const rvalue ref" << endl; }

template <typename T>
void PerfectForward(T&& t) {
  std::cout << "T is a ref type?: " << std::is_reference<T>::value << std::endl;
  std::cout << "T is a lvalue ref type?: " << std::is_lvalue_reference<T>::value
            << std::endl;
  std::cout << "T is a rvalue ref type?: " << std::is_rvalue_reference<T>::value
            << std::endl;

  fun(forward<T>(t));
}

int main() {
  PerfectForward(10);  // call rvalue ref

  int a = 5;
  PerfectForward(a);        // call lvalue ref
  PerfectForward(move(a));  // call rvalue ref

  const int b = 8;
  PerfectForward(b);        // call const lvalue ref
  PerfectForward(move(b));  // call const rvalue ref
}

/*
T is a ref type?: 0
T is a lvalue ref type?: 0
T is a rvalue ref type?: 0
call rvalue ref
T is a ref type?: 1
T is a lvalue ref type?: 1
T is a rvalue ref type?: 0
call lvalue ref
T is a ref type?: 0
T is a lvalue ref type?: 0
T is a rvalue ref type?: 0
call rvalue ref
T is a ref type?: 1
T is a lvalue ref type?: 1
T is a rvalue ref type?: 0
call const lvalue ref
T is a ref type?: 0
T is a lvalue ref type?: 0
T is a rvalue ref type?: 0
call const rvalue ref
*/