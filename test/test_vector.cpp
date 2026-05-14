#include "lib_xcore"
#include "../src/xcore/math_module"
#include <iostream>

void send(...) {}
void delay(...) {}

int main(int argc, char *argv[]) {
  xcore::container::dynamic_array_t<double, 0> vec(8, 8.5);

  for (size_t i = 0; i < 8; ++i) {
    std::cout << vec[i] << " ";
  }

  xcore::stack_t<uint64_t, 4> dst;

  uint64_t src = 0xFF;

  if (dst.find(src) == -1) {
    dst.push(src);
  }

  for (const auto &d: dst) {
    send("huikrfkshkfj", d);
    delay(20);
  }
}
