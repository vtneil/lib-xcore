#include "lib_xcore"
#include <random>
#include <iostream>
#include <unordered_map>

template<typename Engine1, typename Engine2>
void compare_engine(Engine1 &&e1, Engine2 &&e2, const size_t n) {
  for (size_t i = 0; i < n; ++i) {
    std::cout << e1() << " " << e2() << std::endl;
  }
}

template<typename Dist1, typename Dist2,
         typename Engine1, typename Engine2>
void compare_dist(Dist1 &&d1, Dist2 &&d2,
                  Engine1 &&e1, Engine2 &&e2,
                  const size_t n, const size_t m) {
  std::unordered_map<size_t, size_t> counts1;
  std::unordered_map<size_t, size_t> counts2;
  for (size_t i = 0; i < n; ++i) {
    ++counts1[d1(e1)];
    ++counts2[d2(e2)];
  }

  for (size_t i = 0; i < m; ++i) {
    std::cout << i << ": " << counts1[i]
              << ", " << counts2[i]
              << std::endl;
  }
}

int main() {
  const int                          nrolls = 10000;  // number of experiments
  const int                          nstars = 100;    // maximum number of stars to distribute

  xcore::default_random_engine       generator;
  xcore::normal_distribution<double> distribution(5.0, 2.0);

  int                                p[10] = {};

  for (int i = 0; i < nrolls; ++i) {
    double number = distribution(generator);
    if ((number >= 0.0) && (number < 10.0)) ++p[int(number)];
  }

  std::cout << "normal_distribution (5.0,2.0):" << std::endl;

  for (int i = 0; i < 10; ++i) {
    std::cout << i << "-" << (i + 1) << ": ";
    std::cout << std::string(p[i] * nstars / nrolls, '*') << std::endl;
  }

  return 0;
}
