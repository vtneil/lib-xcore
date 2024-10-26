#ifndef STRING_FORMAT_HPP
#define STRING_FORMAT_HPP

#include "core/ported_std.hpp"
#include <cstdio>

template<typename Tp>
ported::enable_if_t<ported::is_integral_v<Tp>, char *> xtostr(Tp value, char *buf, unsigned int radix = 10) {
  if (radix < 2 || radix > 36) {
    buf[0] = '\0';
    return buf;
  }

  constexpr char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char          *p        = buf;
  const bool     negative = value < 0 && ported::is_signed_v<Tp> && radix == 10;

  if (negative) {
    value = -value;
  }

  do {
    *p++ = digits[value % radix];
    value /= radix;
  } while (value > 0);

  if (negative) {
    *p++ = '-';
  }

  *p          = '\0';

  char *start = buf;
  char *end   = p - 1;
  while (start < end) {
    const char temp = *start;
    *start++        = *end;
    *end--          = temp;
  }

  return buf;
}

inline char *dtostr(const double value, const int width, const int precision, char *buffer) {
  char format[10];
// todo merge with xtostr
  sprintf(format, "%%%d.%df", width, precision);
  sprintf(buffer, format, value);

  return buffer;
}

#endif  //STRING_FORMAT_HPP
