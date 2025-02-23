#ifndef LIB_XCORE_MACROS_HPP
#define LIB_XCORE_MACROS_HPP

#ifdef LIB_XCORE_NAMESPACE
#  undef LIB_XCORE_NAMESPACE
#endif
#define LIB_XCORE_NAMESPACE xcore

#ifdef LIB_XCORE_BEGIN_NAMESPACE
#  undef LIB_XCORE_BEGIN_NAMESPACE
#endif
#define LIB_XCORE_BEGIN_NAMESPACE namespace LIB_XCORE_NAMESPACE {

#ifdef LIB_XCORE_END_NAMESPACE
#  undef LIB_XCORE_END_NAMESPACE
#endif
#define LIB_XCORE_END_NAMESPACE }

#endif  //LIB_XCORE_MACROS_HPP
