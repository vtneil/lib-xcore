#ifndef LIB_XCORE_CORE_CUSTOM_NUMERIC_HPP
#define LIB_XCORE_CORE_CUSTOM_NUMERIC_HPP

#include "internal/macros.hpp"
#include "core/ported_std.hpp"
#include <cstdint>

LIB_XCORE_BEGIN_NAMESPACE

template<typename T>
constexpr T max_integral() {
  static_assert(is_integral_v<T>, "Only integral types are supported");
  if constexpr (is_signed_v<T>) {
    return (make_unsigned_t<T>(1) << ((sizeof(T) * 8) - 1)) - 1;
  } else {
    return ~T(0);
  }
}

namespace detail {
  template<uintmax_t Value, size_t Radix>
  struct num_digits_radix {
    static_assert(Radix >= 2 && Radix <= 36, "Base must be in the range 2-36");
    static constexpr size_t value = 1 + num_digits_radix<Value / Radix, Radix>::value;
  };

  template<size_t Radix>
  struct num_digits_radix<0, Radix> {
    static constexpr size_t value = 0;
  };
}  // namespace detail

template<typename T, size_t Radix = 10>
constexpr size_t integral_buffer_size() {
  constexpr bool   is_signed   = is_signed_v<T>;
  constexpr auto   max_value   = static_cast<uintmax_t>(max_integral<T>());
  constexpr size_t digit_count = detail::num_digits_radix<max_value, Radix>::value;
  return 1 + digit_count + (is_signed ? 1 : 0) + 1;
}

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_CUSTOM_NUMERIC_HPP
