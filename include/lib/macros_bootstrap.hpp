#ifndef MACROS_BOOTSTRAP_H
#define MACROS_BOOTSTRAP_H

#define ALIGNED(alignment) __attribute__((aligned(alignment)))

#define INLINE             __inline

#define FORCE_INLINE       INLINE __attribute__((always_inline))

#define NO_INLINE          __attribute__((noinline))

#define VZ(fn)             [&]() -> int { fn; return 0; }

#define EXPECT(a, b)       __builtin_expect(a, b)

#define LIKELY(x)          EXPECT(!!(x), 1)

#define UNLIKELY(x)        EXPECT(!!(x), 0)

#define ASSUME(x)          __builtin_assume(x)

#define UNREACHABLE()      __builtin_unreachable()

#define RESTRICT           __restrict__

#define PURE               __attribute__((pure))

#endif  //MACROS_BOOTSTRAP_H
