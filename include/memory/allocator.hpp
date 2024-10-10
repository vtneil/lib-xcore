#ifndef HPA_2110452_MIN_DOM_SET_MEMORY_H
#define HPA_2110452_MIN_DOM_SET_MEMORY_H

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <new>
#include "memory/generic.hpp"

namespace memory {
  template<template<typename> class allocator_form_t, typename Tp>
  class allocator_t {  // CRTP
  protected:
    using concrete_allocator_t = allocator_form_t<Tp>;

  public:
    FORCE_INLINE static Tp *allocate(const size_t n = 1) noexcept {
      return concrete_allocator_t::impl_allocate(n);
    }

    FORCE_INLINE static void deallocate(Tp *object) noexcept {
      concrete_allocator_t::impl_deallocate(object);
    }
  };

  template<typename Tp>
  class new_allocator_t : public allocator_t<new_allocator_t, Tp> {  // CRTP
  protected:
    friend class allocator_t<new_allocator_t, Tp>;

  protected:
    FORCE_INLINE static Tp *impl_allocate(const size_t n = 1) noexcept {
      return new (std::nothrow) Tp[n];
    }

    FORCE_INLINE static void impl_deallocate(Tp *object) noexcept {
      delete[] object;
    }
  };

  template<typename Tp>
  class malloc_allocator_t : public allocator_t<malloc_allocator_t, Tp> {
  protected:
    friend class allocator_t<malloc_allocator_t, Tp>;

  public:
    FORCE_INLINE static Tp *impl_allocate(const size_t n = 1) noexcept {
      return static_cast<Tp *>(malloc(n * sizeof(Tp)));
    }

    FORCE_INLINE static void impl_deallocate(Tp *object) noexcept {
      free(object);
    }
  };

  template<typename Tp>
  class malloc_clear_allocator_t : public allocator_t<malloc_allocator_t, Tp> {  // CRTP
  protected:
    friend class allocator_t<malloc_allocator_t, Tp>;

  public:
    FORCE_INLINE static Tp *impl_allocate(const size_t n = 1) noexcept {
      void *p_mem = malloc(n * sizeof(Tp));
      memset(p_mem, 0, n * sizeof(Tp));
      return static_cast<Tp *>(p_mem);
    }

    FORCE_INLINE static void impl_deallocate(Tp *object) noexcept {
      free(object);
    }
  };

  template<typename Tp>
  class aligned_allocator_t : public allocator_t<aligned_allocator_t, Tp> {
  protected:
    friend class allocator_t<aligned_allocator_t, Tp>;

  public:
    FORCE_INLINE static Tp *impl_allocate(const size_t n = 1) noexcept {
      static_assert(false, "Not implemented");
      // todo: Implement aligned allocation
      return static_cast<Tp *>(malloc(n * sizeof(Tp)));
    }

    FORCE_INLINE static void impl_deallocate(Tp *object) noexcept {
      static_assert(false, "Not implemented");
      free(object);
    }
  };

  template<typename Tp>
  class unused_allocator_t : public allocator_t<unused_allocator_t, Tp> {
  protected:
    friend class allocator_t<unused_allocator_t, Tp>;

  public:
    template<typename... Args>
    static Tp *impl_allocate(Args...) noexcept {
      static_assert(false, "This allocator should not be used in the program.");
    }

    template<typename... Args>
    static void impl_deallocate(Args...) noexcept {
      static_assert(false, "This allocator should not be used in the program.");
    }
  };

  template<typename Tp>
  using default_allocator_t = malloc_clear_allocator_t<Tp>;

  FORCE_INLINE constexpr bool is_nullptr(const void *pointer) {
    return !pointer;
  }
}  // namespace memory

#endif  //HPA_2110452_MIN_DOM_SET_MEMORY_H
