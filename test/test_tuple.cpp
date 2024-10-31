#include "lib_xcore"
#include <iostream>

int main(int argc, char *argv[]) {
  auto tup = xcore::make_tuple(1, 2, 3);

  std::cout << xcore::get<0>(tup) << std::endl;
  std::cout << xcore::get<1>(tup) << std::endl;
  std::cout << xcore::get<2>(tup) << std::endl;

  const int x[0] = {};

  std::cout << x[0];

  return 0;
}
