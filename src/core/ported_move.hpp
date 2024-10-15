#ifndef PORTED_MOVE_HPP
#define PORTED_MOVE_HPP

#include "core/ported_type_traits.hpp"

namespace ported {
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
}  // namespace ported

#endif  //PORTED_MOVE_HPP
