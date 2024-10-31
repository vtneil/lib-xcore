#ifndef LIB_XCORE_CORE_BUILTINS_BOOTSTRAP_H
#define LIB_XCORE_CORE_BUILTINS_BOOTSTRAP_H

#include "core/macros_bootstrap.hpp"

namespace builtin {
  constexpr unsigned long long next_power_of_two(const unsigned long long x) {
    if (x == 0) return 1;
    if ((x & (x - 1)) == 0) return x;
    return 1ull << (8ull * sizeof(unsigned long long) - __builtin_clzll(x));
  }
}  // namespace builtin

#endif  //LIB_XCORE_CORE_BUILTINS_BOOTSTRAP_H
