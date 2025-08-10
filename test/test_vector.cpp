#include "lib_xcore"
#include "../src/xcore/math_module"
#include <iostream>

int main(int argc, char *argv[]) {
  xcore::container::dynamic_array_t<double, 0> vec(8, 8.5);

  for (size_t i = 0; i < 8; ++i) {
    std::cout << vec[i] << " ";
  }
}
