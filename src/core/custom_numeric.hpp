#ifndef CUSTOM_NUMERIC_HPP
#define CUSTOM_NUMERIC_HPP

#include "core/ported_std.hpp"
#include <cstdint>

namespace utils {
  template<typename T>
  constexpr T max_integral() {
    static_assert(ported::is_integral_v<T>, "Only integral types are supported");
    if constexpr (ported::is_signed_v<T>) {
      return (ported::make_unsigned_t<T>(1) << ((sizeof(T) * 8) - 1)) - 1;
    } else {
      return (ported::make_unsigned_t<T>(1) << (sizeof(T) * 8)) - 1;
    }
  }

  namespace detail {
    template<uintmax_t Value, std::size_t Radix>
    struct num_digits_radix {
      static_assert(Radix >= 2 && Radix <= 36, "Base must be in the range 2-36");
      static constexpr std::size_t value = 1 + num_digits_radix<Value / Radix, Radix>::value;
    };

    template<std::size_t Radix>
    struct num_digits_radix<0, Radix> {
      static constexpr std::size_t value = 0;
    };
  }  // namespace detail

  template<typename T, std::size_t Radix = 10>
  constexpr size_t integral_buffer_size() {
    constexpr bool   is_signed   = ported::is_signed_v<T>;
    constexpr auto   max_value   = static_cast<std::uintmax_t>(utils::max_integral<T>());
    constexpr size_t digit_count = detail::num_digits_radix<max_value, Radix>::value;
    return 1 + digit_count + (is_signed ? 1 : 0) + 1;
  }
}  // namespace utils

#endif  //CUSTOM_NUMERIC_HPP
