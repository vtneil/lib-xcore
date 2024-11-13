#include "lib_xcore"
#include <iostream>

template<typename T>
auto add(const T &a, const T &b) -> T {
  return a + b;
}

template<typename T>
auto mul(const T &a, const T &b) -> T {
  return a * b;
}

int main(int argc, char **argv) {
  auto      div = [](const auto a, const auto b) -> auto { return a / b; };
  const int v   = xcore::pipeline()
                  .run(add<int>, 5, 2)
                  .run(mul<int>, 6)
                  .run(div, 14);
  std::cout << v << std::endl;

  uint8_t data[2];

  xcore::cast_as<uint16_t>(*data) = 256;

  std::cout << xcore::cast_as<const uint16_t>(data) << std::endl;
  std::cout << +data[0] << std::endl;
  std::cout << +data[1] << std::endl;
}
