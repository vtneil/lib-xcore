#ifndef LIB_XCORE_MEMORY_ALLOCATOR_H
#define LIB_XCORE_MEMORY_ALLOCATOR_H

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include "memory/generic.hpp"

namespace xcore::memory {
  template<template<typename> class allocator_form_t, typename Tp>
  class allocator_t {  // CRTP
  protected:
    using concrete_allocator_t = allocator_form_t<Tp>;

  public:
    FORCE_INLINE static constexpr Tp *allocate(const size_t n = 1) noexcept {
      return concrete_allocator_t::impl_allocate(n);
    }

    FORCE_INLINE static constexpr Tp *allocate_inplace(Tp &dst, const size_t n = 1) noexcept {
      return concrete_allocator_t::impl_allocate_inplace(dst, n);
    }

    FORCE_INLINE static constexpr Tp *allocate_inplace(Tp *dst, const size_t n = 1) noexcept {
      return concrete_allocator_t::impl_allocate_inplace(dst, n);
    }

    FORCE_INLINE static constexpr Tp *reallocate(Tp *object, const size_t n) noexcept {
      return concrete_allocator_t::impl_reallocate(object, n);
    }

    FORCE_INLINE static constexpr void deallocate(Tp *object) noexcept {
      concrete_allocator_t::impl_deallocate(object);
    }
  };

  template<typename Tp>
  class new_allocator_t : public allocator_t<new_allocator_t, Tp> {  // CRTP
  protected:
    friend class allocator_t<new_allocator_t, Tp>;

  protected:
    FORCE_INLINE static constexpr Tp *impl_allocate(const size_t n) noexcept {
      return new (std::nothrow) Tp[n];
    }

    FORCE_INLINE static constexpr Tp *impl_allocate_inplace(Tp &dst, const size_t n) noexcept {
      return new (&dst) Tp[n];
    }

    FORCE_INLINE static constexpr Tp *impl_allocate_inplace(Tp *dst, const size_t n) noexcept {
      return new (dst) Tp[n];
    }

    FORCE_INLINE static constexpr Tp *impl_reallocate(Tp *object, const size_t) noexcept {
      return object;  // Unsupported
    }

    FORCE_INLINE static constexpr void impl_deallocate(Tp *object) noexcept {
      delete[] object;
    }
  };

  template<typename Tp>
  class malloc_allocator_t : public allocator_t<malloc_allocator_t, Tp> {
  protected:
    friend class allocator_t<malloc_allocator_t, Tp>;

  public:
    FORCE_INLINE static constexpr Tp *impl_allocate(const size_t n) noexcept {
      return static_cast<Tp *>(malloc(n * sizeof(Tp)));
    }

    FORCE_INLINE static constexpr Tp *impl_allocate_inplace(Tp &dst, const size_t) noexcept {
      return *dst;
    }

    FORCE_INLINE static constexpr Tp *impl_allocate_inplace(Tp *dst, const size_t) noexcept {
      return dst;
    }

    FORCE_INLINE static constexpr Tp *impl_reallocate(Tp *object, const size_t n) noexcept {
      return static_cast<Tp *>(realloc(object, n * sizeof(Tp)));
    }

    FORCE_INLINE static constexpr void impl_deallocate(Tp *object) noexcept {
      free(object);
    }
  };

  template<typename Tp>
  class malloc_clear_allocator_t : public malloc_allocator_t<Tp> {
  public:
    FORCE_INLINE static constexpr Tp *impl_allocate(const size_t n) noexcept {
      void *p_mem = malloc(n * sizeof(Tp));
      memset(p_mem, 0, n * sizeof(Tp));
      return static_cast<Tp *>(p_mem);
    }
  };

  template<typename Tp>
  class aligned_allocator_t : public allocator_t<aligned_allocator_t, Tp> {
  protected:
    friend class allocator_t<aligned_allocator_t, Tp>;

  public:
    FORCE_INLINE static constexpr Tp *impl_allocate(const size_t n = 1) noexcept {
      // todo: Implement aligned allocation
      return nullptr;
    }

    FORCE_INLINE static constexpr void impl_deallocate(Tp *object) noexcept {
    }
  };

  template<typename Tp>
  class unused_allocator_t : public allocator_t<unused_allocator_t, Tp> {
  protected:
    friend class allocator_t<unused_allocator_t, Tp>;

  public:
    template<typename... Args>
    FORCE_INLINE static constexpr Tp *impl_allocate(Args...) noexcept {
      return nullptr;
    }

    template<typename... Args>
    FORCE_INLINE static constexpr void impl_deallocate(Args...) noexcept {
    }
  };

  template<typename Tp>
  using default_allocator_t = malloc_clear_allocator_t<Tp>;

  FORCE_INLINE constexpr bool is_nullptr(const void *pointer) {
    return !pointer;
  }
}  // namespace xcore::memory

namespace xcore {
  using namespace memory;
}

#endif  //LIB_XCORE_MEMORY_ALLOCATOR_H
