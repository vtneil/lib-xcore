#ifndef LIB_XCORE_MEMORY_VIRTUAL_MEMORY_H
#define LIB_XCORE_MEMORY_VIRTUAL_MEMORY_H

#include "internal/macros.hpp"
#include "memory/allocator.hpp"
#include <cstdint>

LIB_XCORE_BEGIN_NAMESPACE

namespace memory {
  /**
     * A virtual region memory region for extended fast region-like allocation
     * using memory pool technique. It should be substantially faster than using malloc/new if
     * the region is pre-allocated for future uses.
     * \n
     * This is just a simple implementation of memory pool designed to be fast.
     * \n
     * Warning: It is the programmer's responsibility to pop the region by deallocating manually in LIFO order.
     * The region will not check for illegal accesses for the sake of performance only but not safety.
     * \n
     * Also, it is easier to clear the region if you know that any memory tied to the region
     * will not be used later in the program.
     *
     * +--------------------------+ <--- bp (begin, high address)
     * |                          |
     * +--------------------------+
     * |                          |
     * |           ...            |
     * |                          |
     * +--------------------------+
     * |                          |
     * +--------------------------+ <--- region_limit (end, low address)
     *
     * @tparam NumBytes             Number of total bytes to be allocated (will be aligned to alignment)
     * @tparam Alignment            Alignment of allocation/de-allocation (should be >= machine's alignment and
     *                              is a power of 2.)
     * @tparam base_allocator_t     Memory allocator to be used to pre-allocate
     */
  template<size_t NumBytes, size_t Alignment = sizeof(void *), template<typename, size_t = 0> class base_allocator_t = malloc_allocator_t>
  class virtual_stack_region_t {
  protected:
    using byte_t                          = uint8_t;
    using byte_allocator                  = base_allocator_t<byte_t, Alignment>;

    static constexpr size_t SizeRequested = NumBytes;
    static constexpr size_t SizeActual    = nearest_alignment<byte_t, Alignment>(NumBytes);
    static constexpr size_t NumAlignments = SizeActual / Alignment;

    byte_t                 *bp;
    byte_t                 *sp;
    byte_t                 *region_limit;

  public:
    virtual_stack_region_t() noexcept : region_limit{byte_allocator::allocate(SizeActual)} {
      bp = region_limit + SizeActual;
      sp = bp;
    }

    ~virtual_stack_region_t() noexcept {
      byte_allocator::deallocate(region_limit);
    }

    [[nodiscard]] FORCE_INLINE constexpr bool valid() const {
      return !is_nullptr(region_limit);
    }

    template<typename T>
    [[nodiscard]] T *construct_ptr(const size_t n = 1) noexcept {
      const T *ptr = allocate_ptr<T>(n);
      return is_nullptr(ptr) ? nullptr : new_allocator_t<T>::allocate_inplace(ptr, n);
    }

    template<typename T>
    [[nodiscard]] T *construct_ptr_unsafe(const size_t n = 1) noexcept {
      return new_allocator_t<T>::allocate_inplace(allocate_ptr_unsafe<T>(n), n);
    }

    template<typename T>
    [[nodiscard]] T &construct(const size_t n = 1) noexcept {
      return *construct_ptr<T>(n);
    }

    template<typename T>
    [[nodiscard]] T &construct_unsafe(const size_t n = 1) noexcept {
      return *construct_ptr_unsafe<T>(n);
    }

    template<typename T>
    [[nodiscard]] T *allocate_ptr(const size_t n = 1) noexcept {
      byte_t *nsp = sp - nearest_alignment<T, Alignment>(n);

      // Check for region overflow
      if (nsp < region_limit) return nullptr;

      sp = nsp;
      return reinterpret_cast<T *>(sp);
    }

    template<typename T>
    [[nodiscard]] T &allocate(const size_t n = 1) noexcept {
      return *allocate_ptr<T>(n);
    }

    template<typename T>
    [[nodiscard]] T *allocate_ptr_unsafe(const size_t n = 1) noexcept {
      sp -= nearest_alignment<T, Alignment>(n);
      return reinterpret_cast<T *>(sp);
    }

    template<typename T>
    [[nodiscard]] T &allocate_unsafe(const size_t n = 1) noexcept {
      return *allocate_ptr_unsafe<T>(n);
    }

    template<typename T>
    void deallocate(const size_t n = 1) noexcept {
      const size_t to_decr = nearest_alignment<T, Alignment>(n);

      ASSUME(bp >= sp);

      if (bp - sp < to_decr) {
        clear();
      } else {
        sp += to_decr;
      }
    }

    template<typename T>
    void deallocate_unsafe(const size_t n = 1) noexcept {
      sp += nearest_alignment<T, Alignment>(n);
    }

    FORCE_INLINE void              clear() noexcept { sp = bp; }

    [[nodiscard]] constexpr size_t size() const noexcept {
      return bp - sp;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
      return bp - region_limit;
    }

    [[nodiscard]] constexpr size_t remaining() const noexcept {
      return sp - region_limit;
    }
  };
}  // namespace memory

using namespace memory;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_MEMORY_VIRTUAL_MEMORY_H
