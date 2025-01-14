#ifndef LIB_XCORE_CORE_PORTED_MOVE_HPP
#define LIB_XCORE_CORE_PORTED_MOVE_HPP

#include "internal/macros.hpp"
#include "core/ported_type_traits.hpp"

LIB_XCORE_BEGIN_NAMESPACE

/**
 * Mimic std::move.
 *
 * @tparam T
 * @param t
 * @return
 */
template<typename T>
FORCE_INLINE constexpr remove_reference_t<T> &&move(T &&t) noexcept { return static_cast<remove_reference_t<T> &&>(t); }

template<typename T>
constexpr T &&forward(remove_reference_t<T> &t) noexcept {
  return static_cast<T &&>(t);
}

template<typename T>
constexpr T &&forward(remove_reference_t<T> &&t) noexcept {
  static_assert(!is_lvalue_reference<T>::value, "cannot forward an rvalue as an lvalue");
  return static_cast<T &&>(t);
}

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_PORTED_MOVE_HPP
