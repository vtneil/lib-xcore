#ifndef HPA_2110452_MIN_DOM_SET_MEMORY_UTILS_H
#define HPA_2110452_MIN_DOM_SET_MEMORY_UTILS_H

#include "core/builtins_bootstrap.hpp"

namespace memory {
  template<typename R = void, typename Tp>
  FORCE_INLINE constexpr auto addressof(Tp &value) {
    return reinterpret_cast<R *>(&value);
  }

  template<typename R = void, typename Tp>
  FORCE_INLINE constexpr auto addressof(const Tp &value) {
    return reinterpret_cast<const R *>(&value);
  }

  template<typename Tp, size_t Alignment>
  FORCE_INLINE constexpr size_t nearest_alignment(const size_t n = 1) {
    return (((n * sizeof(Tp)) + Alignment - 1) / Alignment) * Alignment;
  }

  template<typename Tp, typename AlignT>
  FORCE_INLINE constexpr size_t nearest_alignment(const size_t n = 1) {
    return nearest_alignment<Tp, sizeof(AlignT)>(n);
  }
}  // namespace memory

#endif  //HPA_2110452_MIN_DOM_SET_MEMORY_UTILS_H
