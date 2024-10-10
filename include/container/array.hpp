#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "lib/ported_std.hpp"
#include "lib/types.hpp"
#include <cstdlib>

namespace container {
  template<template<typename, size_t, template<typename> class> class ArrayForm, typename Tp,
           size_t Size>
  class array_container_t {
  private:
    using concrete_array_t = ArrayForm<Tp, Size, memory::unused_allocator_t>;

    concrete_array_t &derived() {
      return static_cast<concrete_array_t &>(*this);
    }

    [[nodiscard]] const concrete_array_t &derived() const {
      return static_cast<const concrete_array_t &>(*this);
    }

  public:
    using reference       = Tp &;
    using const_reference = const Tp &;

    // Algorithm
    [[nodiscard]] constexpr Tp sum() const noexcept {
      Tp accumulator{};

      for (size_t i = 0; i < Size; ++i) {
        accumulator += derived().data[i];
      }

      return accumulator;
    }

    [[nodiscard]] constexpr Tp max() const noexcept {
      Tp max_val = derived().data[0];

      for (size_t i = 1; i < Size; ++i) {
        max_val = derived().data[i] > max_val ? derived().data[i] : max_val;
      }

      return max_val;
    }

    [[nodiscard]] constexpr Tp min() const noexcept {
      Tp min_val = derived().data[0];

      for (size_t i = 1; i < Size; ++i) {
        min_val = derived().data[i] < min_val ? derived().data[i] : min_val;
      }

      return min_val;
    }

    [[nodiscard]] constexpr bool all() const noexcept {
      for (size_t i = 0; i < Size; ++i) {
        if (!derived().data[i]) return false;
      }

      return true;
    }

    [[nodiscard]] constexpr bool any() const noexcept {
      for (size_t i = 0; i < Size; ++i) {
        if (derived().data[i]) return true;
      }

      return false;
    }

    [[nodiscard]] constexpr bool none() const noexcept {
      return !any();
    }

    // Modify
    FORCE_INLINE void clear() noexcept {
      for (size_t i = 0; i < Size; ++i) {
        derived().data[i] = 0;
      }
    }

    constexpr void fill(Tp value, size_t begin, const size_t end) noexcept {
      for (; begin < end; ++begin) {
        derived().data[begin] = value;
      }
    }

    constexpr void fill(Tp value, Tp *begin, Tp *end) noexcept {
      for (; begin < end; ++begin) {
        derived().data[begin] = value;
      }
    }

    // Element access
    [[nodiscard]] FORCE_INLINE constexpr reference operator[](size_t index) noexcept {
      return derived().data[index];
    }

    [[nodiscard]] FORCE_INLINE constexpr const_reference operator[](size_t index) const noexcept {
      return derived().data[index];
    }

    // Iterator
    [[nodiscard]] FORCE_INLINE constexpr Tp *begin() noexcept {
      return memory::addressof<Tp>(derived().data[0]);
    }

    [[nodiscard]] FORCE_INLINE constexpr const Tp *begin() const noexcept {
      return memory::addressof<Tp>(derived().data[0]);
    }

    [[nodiscard]] FORCE_INLINE constexpr Tp *end() noexcept {
      return memory::addressof<Tp>(derived().data[0]) + Size;
    }

    [[nodiscard]] FORCE_INLINE constexpr const Tp *end() const noexcept {
      return memory::addressof<Tp>(derived().data[0]) + Size;
    }

    // Capacity
    [[nodiscard]] FORCE_INLINE constexpr size_t capacity() const noexcept {
      return Size;
    }

    [[nodiscard]] FORCE_INLINE constexpr size_t size() const noexcept {
      return Size;
    }

    [[nodiscard]] FORCE_INLINE constexpr size_t length() const noexcept {
      return size();
    }

    template<traits::has_ostream OStream>
    OStream &operator<<(OStream &os, const array_container_t &arr) {
      os << "{";
      size_t i = 0;
      for (; i < Size - 1; ++i) {
        os << static_cast<Tp>(arr[i]) << ", ";
      }
      os << static_cast<Tp>(arr[i]) << "}";
      return os;
    }
  };
}  // namespace container

#endif  //ARRAY_HPP
