#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "core/ported_std.hpp"
#include "memory.hpp"
#include <cstdlib>

namespace container {
  template<typename Tp, size_t N>
  using c_array = Tp[N];

  namespace utils {
    template<size_t Capacity>
    constexpr size_t cyclic(const size_t current) { return current % Capacity; }
  }  // namespace utils

  namespace detail {
    template<template<typename, size_t, template<typename> class> class ArrayForm, typename Tp, size_t Size>
    struct array_container_t {
    private:
      using concrete_array_t = ArrayForm<Tp, Size, memory::unused_allocator_t>;

      concrete_array_t *derived() {
        return static_cast<concrete_array_t *>(this);
      }

      [[nodiscard]] const concrete_array_t *derived() const {
        return static_cast<const concrete_array_t *>(this);
      }

      [[nodiscard]] constexpr Tp       *get_derived_data() noexcept { return static_cast<Tp *>(*derived()); }

      [[nodiscard]] constexpr const Tp *get_derived_data() const noexcept { return static_cast<const Tp *>(*derived()); }

    public:
      using reference       = Tp &;
      using const_reference = const Tp &;

      // Algorithm
      [[nodiscard]] constexpr Tp sum() const noexcept {
        Tp accumulator{};

        for (size_t i = 0; i < Size; ++i) {
          accumulator += get_derived_data()[i];
        }

        return accumulator;
      }

      [[nodiscard]] constexpr Tp max() const noexcept {
        Tp max_val = get_derived_data()[0];

        for (size_t i = 1; i < Size; ++i) {
          max_val = get_derived_data()[i] > max_val ? get_derived_data()[i] : max_val;
        }

        return max_val;
      }

      [[nodiscard]] constexpr Tp min() const noexcept {
        Tp min_val = get_derived_data()[0];

        for (size_t i = 1; i < Size; ++i) {
          min_val = get_derived_data()[i] < min_val ? get_derived_data()[i] : min_val;
        }

        return min_val;
      }

      [[nodiscard]] constexpr bool all() const noexcept {
        for (size_t i = 0; i < Size; ++i) {
          if (!get_derived_data()[i]) return false;
        }

        return true;
      }

      [[nodiscard]] constexpr bool any() const noexcept {
        for (size_t i = 0; i < Size; ++i) {
          if (get_derived_data()[i]) return true;
        }

        return false;
      }

      [[nodiscard]] constexpr bool none() const noexcept {
        return !any();
      }

      // Modify
      FORCE_INLINE void clear() noexcept {
        for (size_t i = 0; i < Size; ++i) {
          get_derived_data()[i] = 0;
        }
      }

      constexpr void fill(Tp value, size_t begin, const size_t end) noexcept {
        for (; begin < end; ++begin) {
          get_derived_data()[begin] = value;
        }
      }

      constexpr void fill(Tp value, Tp *begin, Tp *end) noexcept {
        for (; begin < end; ++begin) {
          get_derived_data()[begin] = value;
        }
      }

      // Element access
      [[nodiscard]] FORCE_INLINE constexpr reference operator[](size_t index) noexcept {
        return get_derived_data()[index];
      }

      [[nodiscard]] FORCE_INLINE constexpr const_reference operator[](size_t index) const noexcept {
        return get_derived_data()[index];
      }

      // Iterator
      [[nodiscard]] FORCE_INLINE constexpr Tp *begin() noexcept {
        return memory::addressof<Tp>(get_derived_data()[0]);
      }

      [[nodiscard]] FORCE_INLINE constexpr const Tp *begin() const noexcept {
        return memory::addressof<Tp>(get_derived_data()[0]);
      }

      [[nodiscard]] FORCE_INLINE constexpr Tp *end() noexcept {
        return memory::addressof<Tp>(get_derived_data()[0]) + Size;
      }

      [[nodiscard]] FORCE_INLINE constexpr const Tp *end() const noexcept {
        return memory::addressof<Tp>(get_derived_data()[0]) + Size;
      }

      // Capacity

      [[nodiscard]] FORCE_INLINE constexpr size_t size() const noexcept {
        return Size;
      }

      [[nodiscard]] FORCE_INLINE constexpr size_t length() const noexcept {
        return size();
      }

      // Dynamic array capability
      void dynamic_resize(const size_t) {
        // NOP
      }

      void dynamic_clear() {
        // NOP
      }
    };
  }  // namespace detail

  /**
   * Static region-allocated data container
   */
  template<typename Tp, size_t Size, template<typename> class = memory::unused_allocator_t>
  struct array_t : detail::array_container_t<array_t, Tp, Size> {
  protected:
    c_array<Tp, Size> arr_;

  public:
    [[nodiscard]] FORCE_INLINE constexpr operator Tp *() noexcept {  // Implicit
      return arr_;
    }

    [[nodiscard]] FORCE_INLINE constexpr operator const Tp *() const noexcept {  // Implicit
      return arr_;
    }
  };

  /**
   * Static heap-allocated data container
   */
  template<typename Tp, size_t Size, template<typename> class BaseAllocator = memory::default_allocator_t>
  struct heap_array_t : detail::array_container_t<heap_array_t, Tp, Size> {
  private:
    using array_type      = array_t<Tp, Size>;
    using array_allocator = BaseAllocator<array_type>;

  protected:
    array_type *arr_;

  public:
    // Constructor
    heap_array_t() : arr_(array_allocator::allocate(1)) {}

    // Destructor
    ~heap_array_t() {
      if (arr_) {
        array_allocator::deallocate(arr_);
        arr_ = nullptr;
      }
    }

    [[nodiscard]] FORCE_INLINE constexpr operator Tp *() noexcept {  // Implicit
      return *arr_;                                                  // Implicit
    }

    [[nodiscard]] FORCE_INLINE constexpr operator const Tp *() const noexcept {  // Implicit
      return *arr_;                                                              // Implicit
    }
  };

  /**
  * Simple heap-allocated dynamic array.
  * This is not a ported version of std::vector.
  * NOT INTENDED FOR DIRECT USAGE.
  */
  template<typename Tp, size_t, template<typename> class BaseAllocator = memory::default_allocator_t>
  struct dynamic_array_t : detail::array_container_t<dynamic_array_t, Tp, 0> {
  private:
    using array_allocator = BaseAllocator<Tp>;

  protected:
    Tp    *arr_;
    size_t size_;

  public:
    // Constructor
    dynamic_array_t()
        : arr_(nullptr), size_(0) {}

    explicit dynamic_array_t(size_t initial_size)
        : arr_(array_allocator::allocate(initial_size)), size_(initial_size) {}

    // Destructor
    ~dynamic_array_t() {
      this->dynamic_clear();
    }

    // Capacity

    [[nodiscard]] FORCE_INLINE constexpr size_t size() const noexcept {
      return size_;
    }

    void dynamic_resize(const size_t new_size) {
      if (new_size == size_) return;

      arr_  = array_allocator::reallocate(arr_, new_size);
      size_ = new_size;
    }

    void dynamic_clear() {
      if (arr_) {
        array_allocator::deallocate(arr_);
        arr_  = nullptr;
        size_ = 0;
      }
    }

    [[nodiscard]] FORCE_INLINE constexpr operator Tp *() noexcept {  // Implicit
      return *arr_;                                                  // Implicit
    }

    [[nodiscard]] FORCE_INLINE constexpr operator const Tp *() const noexcept {  // Implicit
      return *arr_;                                                              // Implicit
    }
  };
}  // namespace container

#endif  //ARRAY_HPP
