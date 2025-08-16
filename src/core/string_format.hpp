#ifndef LIB_XCORE_CORE_STRING_FORMAT_HPP
#define LIB_XCORE_CORE_STRING_FORMAT_HPP

#include "internal/macros.hpp"
#include "core/ported_std.hpp"
#include "core/dtostrf.hpp"
#include <cstdio>

#define XCORE_SPRINTF_FLOAT 0

LIB_XCORE_BEGIN_NAMESPACE

/**
* Overload: integral (signed/unsigned char, short, int, long, long long) to string
*/
template<typename Tp>
enable_if_t<is_integral_v<Tp>, char *> xtostr(Tp value, char *buf, unsigned int radix = 10) {
  if (radix < 2 || radix > 36) {
    buf[0] = '\0';
    return buf;
  }

  constexpr char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char          *p        = buf;
  const bool     negative = value < 0 && is_signed_v<Tp> && radix == 10;

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

  *p = '\0';

  char *start = buf;
  char *end   = p - 1;
  while (start < end) {
    const char temp = *start;
    *start++        = *end;
    *end--          = temp;
  }

  return buf;
}

/**
* Overload: floating points (float, double) to string
*/
template<typename Tp>
enable_if_t<is_floating_point_v<Tp>, char *> xtostr(Tp value, char *buf, const int width, const int precision) {
#if defined(XCORE_SPRINTF_FLOAT) && XCORE_SPRINTF_FLOAT == 1
  char format[10];
  sprintf(format, "%%%d.%df", width, precision);
  sprintf(buf, format, value);
#else
  dtostrf(value, width, precision, buf);
#endif
  return buf;
}

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_STRING_FORMAT_HPP
