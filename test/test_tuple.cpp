#include "embedded_cpp"
#include <iostream>

int main(int argc, char *argv[]) {
  auto tup = ported::make_tuple(1, 2, 3);

  std::cout << ported::get<0>(tup) << std::endl;
  std::cout << ported::get<1>(tup) << std::endl;
  std::cout << ported::get<2>(tup) << std::endl;

  return 0;
}
