#ifndef DTOSTRF_HPP
#define DTOSTRF_HPP

#include "internal/macros.hpp"
#include "core/ported_std.hpp"

LIB_XCORE_BEGIN_NAMESPACE

char *dtostrf(double val, signed char width, unsigned char prec, char *sout);

LIB_XCORE_END_NAMESPACE

#endif  //DTOSTRF_HPP
