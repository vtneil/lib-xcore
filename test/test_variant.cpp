#include "lib_xcore"
#include <iostream>
#include <cassert>

int main(int argc, char *argv[]) {
  xcore::variant<unsigned, int> v1;

  assert(xcore::holds_alternative<int>(v1));
  assert(!xcore::holds_alternative<float>(v1));
  assert(!xcore::holds_alternative<double>(v1));
  assert(xcore::holds_alternative<unsigned>(v1));

  v1.emplace<unsigned>(-1);

  std::cout << xcore::get<int>(v1) << std::endl;
  std::cout << xcore::get<unsigned>(v1) << std::endl;

  return 0;
}
