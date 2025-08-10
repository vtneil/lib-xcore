#ifndef LIB_XCORE_CORE_MOVE_HPP
#define LIB_XCORE_CORE_MOVE_HPP

#include "internal/macros.hpp"

#include "core/ported_invoke.hpp"
#include "core/ported_refwrap.hpp"
#include "core/ported_type_traits.hpp"
#include "core/ported_move.hpp"

#include "core/macros_bootstrap.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstring>

LIB_XCORE_BEGIN_NAMESPACE

#if defined(XCORE_USE_FLOAT)
#  define XCORE_REAL_T float
#elif defined(XCORE_USE_DOUBLE)
#  define XCORE_REAL_T double
#elif defined(XCORE_USE_LONG_DOUBLE)
#  define XCORE_REAL_T = long double;
#else
#  define XCORE_REAL_T double
#  define XCORE_USE_DOUBLE
#endif

using real_t = XCORE_REAL_T;

#if !defined(XCORE_FLOAT_THRESHOLD)
#  define XCORE_FLOAT_THRESHOLD 1.e-10
#endif


template<typename T>
FORCE_INLINE constexpr const T &min(const T &v) { return v; }

template<typename T>
FORCE_INLINE constexpr const T &min(const T &a, const T &b) { return a < b ? a : b; }

template<typename T, typename... Ts>
FORCE_INLINE constexpr const T &min(const T &a, const T &b, const Ts &...args) {
  return min(min(a, b), args...);
}

template<typename T>
FORCE_INLINE constexpr const T &max(const T &v) { return v; }

template<typename T>
FORCE_INLINE constexpr const T &max(const T &a, const T &b) { return a > b ? a : b; }

template<typename T, typename... Ts>
FORCE_INLINE constexpr const T &max(const T &a, const T &b, const Ts &...args) {
  return max(max(a, b), args...);
}

/**
  * Mimic std::swap
  *
  * @tparam T
  * @param a
  * @param b
  */
template<typename T>
void swap(T &a, T &b) noexcept {
  T tmp = move(a);
  a     = move(b);
  b     = move(tmp);
}

/**
  * Mimic std::fill
  *
  * @tparam ForwardIt
  * @tparam T
  * @param first
  * @param last
  * @param value
  */
template<typename ForwardIt, typename T>
constexpr void fill(ForwardIt first, ForwardIt last, const T &value) {
  for (; first != last; static_cast<void>(++first)) *first = value;
}

/**
  * Mimic std::copy
  *
  * @tparam InputIt
  * @tparam OutputIt
  * @param first
  * @param last
  * @param d_first
  * @return
  */
template<typename InputIt, typename OutputIt>
constexpr OutputIt copy(InputIt first, InputIt last, OutputIt d_first) {
  for (; first != last; static_cast<void>(++first), static_cast<void>(++d_first)) *d_first = *first;
  return d_first;
}

/**
  * Mimic std::move
  *
  * @tparam InputIt
  * @tparam OutputIt
  * @param first
  * @param last
  * @param d_first
  * @return
  */
template<typename InputIt, typename OutputIt>
constexpr OutputIt move(InputIt first, InputIt last, OutputIt d_first) {
  for (; first != last; static_cast<void>(++first), static_cast<void>(++d_first)) *d_first = move(*first);
  return d_first;
}

template<size_t...>
struct index_sequence {};

template<size_t N, size_t... Is>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...> {};

template<size_t... Is>
struct make_index_sequence<0, Is...> : index_sequence<Is...> {};

template<class... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_MOVE_HPP
