#ifndef LIB_XCORE_CORE_PORTED_PAIR_HPP
#define LIB_XCORE_CORE_PORTED_PAIR_HPP

#include "internal/macros.hpp"
#include "./ported_type_traits.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<typename T1, typename T2>
struct pair {
  T1 first;
  T2 second;

  // Default constructor
  template<typename = enable_if_t<conjunction<is_default_constructible<T1>, is_default_constructible<T2>>::value>>
  constexpr pair() : first(T1{}), second(T2{}) {}

  // Copy constructor
  constexpr pair(const pair &other) = default;

  // Move constructor
  constexpr pair(pair &&other) noexcept = default;

  // Copy objects constructor
  constexpr pair(const T1 &first, const T2 &second) : first(first), second(second) {}

  // Move objects constructor
  constexpr pair(T1 &&first, T2 &&second) noexcept : first(forward<T1>(first)), second(forward<T2>(second)) {}

  // Copy assignment
  pair &operator=(const pair &other) = default;

  // Move assignment
  pair &operator=(pair &&other) noexcept = default;

  // Equality and ordering operators
  constexpr bool operator==(const pair &other) const {
    return first == other.first && second == other.second;
  }

  constexpr bool operator!=(const pair &other) const { return !(*this == other); }
  constexpr bool operator<(const pair &other) const {
    return first < other.first || (!(other.first < first) && second < other.second);
  }

  constexpr bool operator>(const pair &other) const { return other < *this; }
  constexpr bool operator<=(const pair &other) const { return !(other < *this); }
  constexpr bool operator>=(const pair &other) const { return !(*this < other); }
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
