#ifndef LIB_XCORE_BITMAP_ALLOCATOR_HPP
#define LIB_XCORE_BITMAP_ALLOCATOR_HPP

#include "internal/macros.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include "memory.hpp"

#include "core/ported_type_traits.hpp"
#include "container/bitset.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace memory {
  /**
   * An implementation of simple static bitmap allocator using bitset and fixed-size memory arena.
   *
   * **WARNING: SAFETY, OWNERSHIP, etc. IS NOT GUARANTEED!**
   *
   * **RECOMMEND: USE IN CONJUNCTION WITH OTHER SAFE WRAPPERS IF YOU WANT**
   *
   * @tparam Tp Element Type
   * @tparam Capacity Storage Capacity
   */
  template<typename Tp, size_t Capacity>
  class bitmap_allocator {
    using pointer       = Tp *;
    using const_pointer = const Tp *;

  protected:
    aligned_storage<sizeof(Tp), alignof(Tp)> arena_[Capacity]{};
    bitset_t<Capacity>                       book_{};  // False = free, True = occupied
    size_t                                   size_{};

  public:
    bitmap_allocator() = default;

    /**
     * Acquires a pointer to a new allocation slot in the arena if capacity allows.
     *
     * @return A pointer to the newly allocated slot in the arena. Returns nullptr
     *         if the number of currently allocated slots reaches the capacity.
     *
     * The function checks if the current size of the allocations is less than
     * the allowed capacity. If the capacity is not exceeded, it locates the next
     * available allocation index using the bitmap allocator's "find_first_false"
     * method. It then marks the located index as occupied using "book_new" and
     * calculates the pointer to the allocated slot by adding the index to the
     * arena's base pointer.
     */
    pointer acquire() {
      if (size() >= capacity())
        return nullptr;

      const size_t index = book_.find_first_false();
      book_new(index);

      return arena_base() + index;
    }

    /**
     * @brief Acquires a new pointer from the arena if capacity allows.
     *
     * This function attempts to allocate a new pointer from the memory arena. It checks if
     * the current size has reached the capacity. If the capacity has been reached, it returns
     * a nullptr. Otherwise, it determines the next available index using the 'book_' object,
     * marks this index as used, and returns a pointer to the corresponding memory in the arena.
     *
     * @return const_pointer Pointer to the allocated memory if successful, or nullptr if the capacity has been reached.
     */
    const_pointer acquire() const {
      if (size() >= capacity())
        return nullptr;

      const size_t index = book_.find_first_false();
      book_new(index);

      return arena_base() + index;
    }

    /**
     * @brief Releases a previously acquired object back to the allocator.
     *
     * This function marks the specified object as available in the allocator,
     * allowing it to be reused. It verifies the validity of the pointer before
     * attempting to release it. If the pointer is invalid or was not previously
     * acquired, the function performs no operation.
     *
     * @param ptr The pointer to the object to be released back into the allocator.
     */
    void release(const_pointer ptr) {
      const size_t index = ptr - arena_base();

      if (index >= Capacity ||
          book_[index] == false)
        return;

      book_delete(index);
    }

    /**
     * @brief Retrieves the size of a container or collection.
     *
     * @details This function returns the current size of the container or collection
     * as a constant expression, allowing for compile-time evaluation if the size is determined
     * at compile time.
     *
     * @return The size of the container or collection as a size_t.
     *
     * @note The `[[nodiscard]]` attribute is used to indicate that the return value should not be ignored.
     */
    [[nodiscard]] constexpr size_t size() const {
      return size_;
    }

    /**
     * @brief Returns the compile-time defined capacity of the container.
     *
     * @return The constant capacity value of the container.
     * @note This function is marked as [[nodiscard]], indicating the
     *       return value should not be ignored.
     */
    [[nodiscard]] constexpr size_t capacity() const {
      return Capacity;
    }

  protected:
    void book_new(const size_t index) {
      book_[index] = true;
      ++size_;
    }

    void book_delete(const size_t index) {
      book_[index] = false;
      --size_;
    }

    pointer arena_base() {
      return reinterpret_cast<pointer>(arena_);
    }

    const_pointer arena_base() const {
      return reinterpret_cast<const_pointer>(arena_);
    }
  };
}  // namespace memory

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_BITMAP_ALLOCATOR_HPP
