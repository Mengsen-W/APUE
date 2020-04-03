/*
 * @Author: Mengsen.Wang
 * @Date: 2020-04-03 07:56:19
 * @Last Modified by: Mengsen.Wang
 * @Last Modified time: 2020-04-03 08:23:13
 */

#include <iostream>
#include <string>

class Obj {
 public:
  Obj() { std::cout << "Obj::Obj(){}" << std::endl; }
  Obj(const std::string& str, const int& i) : _str(str), _i(i) {
    std::cout << "const string& const int&" << std::endl;
  }
  Obj(std::string& str, int& i) : _str(str), _i(i) {
    std::cout << "string&, int&" << std::endl;
  }
  Obj(const Obj& obj) : _str(obj._str), _i(obj._i) {
    std::cout << "const Obj&" << std::endl;
  }
  ~Obj() { std::cout << "Obj::~Obj(){}" << std::endl; }

 private:
  std::string _str;
  int _i;
};

// void getInstance(const int& arg) {
//   std::cout << "call const int" << std::endl;
//   return;
// }

int main() {
  // const int a = 10;
  // int b = 10;
  // getInstance(a);
  // getInstance(b);
  // getInstance(5);
  Obj obj1;
  Obj obj2("wms", 1);
  Obj obj3{"w", 2};
  std::string a1 = "wms";
  int b1 = 1;
  Obj obj4(a1, b1);
  Obj obj5(std::move(a1), std::move(b1));
  const std::string a2 = "wms";
  const int b2 = 1;
  Obj obj6{a2, b2};
  Obj obj7(std::move(a2), std::move(b2));
  return 0;
}

/*
 * 所有的右值都变成了 const& const& 左值也是
 * 只有显式的声明非const左值才会被导向非const
 */