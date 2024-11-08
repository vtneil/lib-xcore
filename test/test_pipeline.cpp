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
  std::cout << v;
}
