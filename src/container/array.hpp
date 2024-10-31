#ifndef LIB_XCORE_CONTAINER_ARRAY_HPP
#define LIB_XCORE_CONTAINER_ARRAY_HPP

#include "core/ported_std.hpp"
#include "memory.hpp"
#include <cstdlib>

namespace xcore::container {
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
      using concrete_array_t = ArrayForm<Tp, Size, unused_allocator_t>;

      constexpr concrete_array_t *derived() {
        return static_cast<concrete_array_t *>(this);
      }

      [[nodiscard]] constexpr const concrete_array_t *derived() const {
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
        return addressof<Tp>(get_derived_data()[0]);
      }

      [[nodiscard]] FORCE_INLINE constexpr const Tp *begin() const noexcept {
        return addressof<Tp>(get_derived_data()[0]);
      }

      [[nodiscard]] FORCE_INLINE constexpr const Tp *cbegin() const noexcept {
        return this->begin();
      }

      [[nodiscard]] FORCE_INLINE constexpr Tp *end() noexcept {
        return addressof<Tp>(get_derived_data()[0]) + Size;
      }

      [[nodiscard]] FORCE_INLINE constexpr const Tp *end() const noexcept {
        return addressof<Tp>(get_derived_data()[0]) + Size;
      }

      [[nodiscard]] FORCE_INLINE constexpr const Tp *cend() const noexcept {
        return this->end();
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
  template<typename Tp, size_t Size, template<typename> class = unused_allocator_t>
  struct array_t : detail::array_container_t<array_t, Tp, Size> {
  protected:
    c_array<Tp, Size> arr_ = {};

  public:
    // Default Constructor
    constexpr array_t() = default;

    // Copy Constructor
    constexpr array_t(const array_t &other) {
      copy(other.arr_, other.arr_ + Size, arr_);
    }

    // Move Constructor
    constexpr array_t(array_t &&other) noexcept {
      move(other.arr_, other.arr_ + Size, arr_);
    }

    // Fill constructor
    explicit constexpr array_t(const Tp &fill) : array_t(fill, make_index_sequence<Size>{}) {}

  private:
    template<size_t... I>
    explicit constexpr array_t(const Tp &fill, index_sequence<I...>)
        : arr_{((void) I, fill)...} {}

  public:
    // Copy Assignment Operator
    constexpr array_t &operator=(const array_t &other) {
      if (this != &other) {
        copy(other.arr_, other.arr_ + Size, arr_);
      }
      return *this;
    }

    // Move Assignment Operator
    constexpr array_t &operator=(array_t &&other) noexcept {
      if (this != &other) {
        move(other.arr_, other.arr_ + Size, arr_);
      }
      return *this;
    }

    // Destructor
    ~array_t() = default;

    // Implicit
    [[nodiscard]] FORCE_INLINE constexpr operator Tp *() noexcept {
      return arr_;
    }

    // Implicit
    [[nodiscard]] FORCE_INLINE constexpr operator const Tp *() const noexcept {
      return arr_;
    }
  };

  /**
   * Static heap-allocated data container
   */
  template<typename Tp, size_t Size, template<typename> class BaseAllocator = default_allocator_t>
  struct heap_array_t : detail::array_container_t<heap_array_t, Tp, Size> {
  private:
    using array_type      = array_t<Tp, Size>;
    using array_allocator = BaseAllocator<array_type>;

  protected:
    array_type *arr_;

  public:
    // Default Constructor
    constexpr heap_array_t() : arr_(array_allocator::allocate(1)) {}

    // Copy Constructor
    constexpr heap_array_t(const heap_array_t &other) : arr_(array_allocator::allocate(1)) {
      copy(other.arr_->begin(), other.arr_->end(), arr_->begin());
    }

    // Move Constructor
    constexpr heap_array_t(heap_array_t &&other) noexcept : arr_(other.arr_) {
      other.arr_ = nullptr;
    }

    // Fill constructor
    explicit constexpr heap_array_t(const Tp &fill) : heap_array_t() {
      new (arr_) array_type(fill);
    }

    // Destructor
    ~heap_array_t() {
      if (arr_) {
        array_allocator::deallocate(arr_);
        arr_ = nullptr;
      }
    }

    // Copy Assignment Operator
    heap_array_t &operator=(const heap_array_t &other) {
      if (this != &other) {
        if (!arr_)
          arr_ = array_allocator::allocate(1);
        copy(other.arr_->begin(), other.arr_->end(), arr_->begin());
      }
      return *this;
    }

    // Move Assignment Operator
    heap_array_t &operator=(heap_array_t &&other) noexcept {
      if (this != &other) {
        if (arr_)
          array_allocator::deallocate(arr_);
        arr_       = other.arr_;
        other.arr_ = nullptr;
      }
      return *this;
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
  template<typename Tp, size_t, template<typename> class BaseAllocator = default_allocator_t>
  struct dynamic_array_t : detail::array_container_t<dynamic_array_t, Tp, 0> {
  private:
    using array_allocator = BaseAllocator<Tp>;

  protected:
    Tp    *arr_;
    size_t size_;

  public:
    // Default Constructor
    constexpr dynamic_array_t()
        : arr_(nullptr), size_(0) {}

    // Constructor with initial size
    explicit constexpr dynamic_array_t(const size_t initial_size)
        : arr_(array_allocator::allocate(initial_size)), size_(initial_size) {}

    // Copy Constructor
    constexpr dynamic_array_t(const dynamic_array_t &other)
        : arr_(array_allocator::allocate(other.size_)), size_(other.size_) {
      copy(other.arr_, other.arr_ + size_, arr_);
    }

    // Move Constructor
    constexpr dynamic_array_t(dynamic_array_t &&other) noexcept
        : arr_(other.arr_), size_(other.size_) {
      other.arr_  = nullptr;
      other.size_ = 0;
    }

    // Fill Constructor
    constexpr dynamic_array_t(const size_t initial_size, const Tp &fill) : dynamic_array_t(initial_size) {
      fill(arr_, arr_ + initial_size, fill);
    }

    // Copy Assignment Operator
    dynamic_array_t &operator=(const dynamic_array_t &other) {
      if (this != &other) {
        if (arr_)
          array_allocator::deallocate(arr_);
        size_ = other.size_;
        arr_  = array_allocator::allocate(size_);
        copy(other.arr_, other.arr_ + size_, arr_);
      }
      return *this;
    }

    // Move Assignment Operator
    dynamic_array_t &operator=(dynamic_array_t &&other) noexcept {
      if (this != &other) {
        if (arr_)
          array_allocator::deallocate(arr_);
        arr_        = other.arr_;
        size_       = other.size_;
        other.arr_  = nullptr;
        other.size_ = 0;
      }
      return *this;
    }

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
      return arr_;                                                   // Implicit
    }

    [[nodiscard]] FORCE_INLINE constexpr operator const Tp *() const noexcept {  // Implicit
      return arr_;                                                               // Implicit
    }
  };
}  // namespace xcore::container

namespace xcore {
  using namespace container;
}

#endif  //LIB_XCORE_CONTAINER_ARRAY_HPP
