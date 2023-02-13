#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <vector>

#include "thread_pool.h"
using namespace std;

class Test {
public:
  Test(int i) : Testi(i) {}

  int test(int i) {
    cout << "int test(int i)" << i << endl;
    cout << "Testi = " << Testi << endl;
    return i;
  }

  int Testi;
};

int TestFunc(int i, double& b) {
  cout << "int TestFunc(int i, double b)" << endl;
  return i;
}

int main() {
  ThreadPool p(4);
  Test t(1000);
  int b = 10;
  double c = 100.0;
  p.AddTask(&Test::test, &t, b);
  p.AddTask(&Test::test, t, b);
  p.AddTask(&Test::Testi, &t);
  p.AddTask(&Test::Testi, t);
  p.AddTask(&TestFunc, b, std::ref(c));

  std::cout << "1 :" << p.Submit(&Test::test, &t, b).get() << std::endl;
  std::cout << "2 :" << p.Submit(&Test::test, t, b).get() << std::endl;
  std::cout << "3 :" << p.Submit(&Test::Testi, &t).get() << std::endl;
  std::cout << "4 :" << p.Submit(&Test::Testi, t).get() << std::endl;
  std::cout << "5 :" << p.Submit(&TestFunc, b, std::ref(c)).get() << std::endl;

  return 0;
}
