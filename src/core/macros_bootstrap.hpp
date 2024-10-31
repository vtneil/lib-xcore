#ifndef LIB_XCORE_CORE_MACROS_BOOTSTRAP_H
#define LIB_XCORE_CORE_MACROS_BOOTSTRAP_H

#ifndef PACKED
#  define PACKED __attribute__((packed))
#endif

#ifndef ALIGNED
#  define ALIGNED(alignment) __attribute__((aligned(alignment)))
#endif

#ifndef INLINE
#  define INLINE __inline
#endif

#ifndef FORCE_INLINE
#  define FORCE_INLINE INLINE __attribute__((always_inline))
#endif

#ifndef NO_INLINE
#  define NO_INLINE __attribute__((noinline))
#endif

#ifndef VZ
#  define VZ(fn) [&]() -> int { fn; return 0; }
#endif

#if __has_builtin(__builtin_expect)

#  ifndef EXPECT
#    define EXPECT(a, b) __builtin_expect(a, b)
#  endif

#  ifndef LIKELY
#    define LIKELY(x) EXPECT(!!(x), 1)
#  endif

#  ifndef UNLIKELY
#    define UNLIKELY(x) EXPECT(!!(x), 0)
#  endif

#else

#  ifndef EXPECT
#    define EXPECT(a, b)
#  endif

#  ifndef LIKELY
#    define LIKELY(x)
#  endif

#  ifndef UNLIKELY
#    define UNLIKELY(x)
#  endif

#endif

#if __has_builtin(__builtin_assume)

#  ifndef ASSUME
#    define ASSUME(x) __builtin_assume(x)
#  endif

#else

#  ifndef ASSUME
#    define ASSUME(x)
#  endif

#endif

#if __has_builtin(__builtin_unreachable)

#  ifndef UNREACHABLE
#    define UNREACHABLE() __builtin_unreachable()
#  endif

#else

#  ifndef UNREACHABLE
#    define UNREACHABLE()
#  endif

#endif

#ifndef RESTRICT
#  define RESTRICT __restrict__
#endif

#ifndef PURE
#  define PURE __attribute__((pure))
#endif

#endif  //LIB_XCORE_CORE_MACROS_BOOTSTRAP_H
