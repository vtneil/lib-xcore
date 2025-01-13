#ifndef LIB_XCORE_CORE_PORTED_PAIR_HPP
#define LIB_XCORE_CORE_PORTED_PAIR_HPP

#include "internal/macros.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<typename T1, typename T2>
struct pair {
  T1 first;
  T2 second;

  // Default constructor
  pair() = default;

  // Copy constructor
  pair(const pair &other) = default;

  // Move constructor
  pair(pair &&other) noexcept = default;

  // Copy objects constructor
  pair(const T1 &first, const T2 &second) : first(first), second(second) {}

  // Move objects constructor
  pair(T1 &&first, T2 &&second) noexcept : first(move(first)), second(move(second)) {}

  // Copy assignment
  pair &operator=(const pair &other) = default;

  // Move assignment
  pair &operator=(pair &&other) noexcept = default;
};

template<typename T1, typename T2>
constexpr pair<T1, T2> make_pair(const T1 &t1, const T2 &t2) {
  return pair<T1, T2>(t1, t2);
}

template<typename T1, typename T2>
constexpr pair<T1, T2> make_pair(T1 &&t1, T2 &&t2) {
  return pair<T1, T2>(forward<T1>(t1), forward<T2>(t2));
}

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_PORTED_PAIR_HPP
