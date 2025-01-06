#ifndef LIB_XCORE_CORE_BUILTINS_BOOTSTRAP_H
#define LIB_XCORE_CORE_BUILTINS_BOOTSTRAP_H

#include "internal/macros.hpp"
#include "core/macros_bootstrap.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace builtin {
  constexpr unsigned long long next_power_of_two(const unsigned long long x) {
    if (x == 0) return 1;
    if ((x & (x - 1)) == 0) return x;
    return 1ull << (8ull * sizeof(unsigned long long) - __builtin_clzll(x));
  }
}  // namespace builtin

template<typename R, typename T>
constexpr R &cast_as(T &ptr) {
  return *reinterpret_cast<R *>(&ptr);
}

template<typename R, typename T>
constexpr const R &cast_as(const T &ref) {
  return *reinterpret_cast<const R *>(&ref);
}

template<typename R, typename T>
constexpr R &cast_as(T *ptr) {
  return *reinterpret_cast<R *>(ptr);
}

template<typename R, typename T>
constexpr const R &cast_as(const T *ptr) {
  return *reinterpret_cast<const R *>(ptr);
}

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_BUILTINS_BOOTSTRAP_H
