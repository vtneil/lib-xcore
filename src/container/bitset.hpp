#ifndef LIB_XCORE_CONTAINER_BITSET_HPP
#define LIB_XCORE_CONTAINER_BITSET_HPP

#include "memory.hpp"
#include <cstdint>

namespace xcore::container {
  template<size_t Nb, typename WordT = int>
  struct bitset_t {
  private:
    using byte_t                              = int8_t;

    static constexpr size_t Alignment         = sizeof(WordT);
    static constexpr size_t NumBytes          = (Nb + 8 - 1) / 8;
    static constexpr size_t SizeActual        = nearest_alignment<byte_t, Alignment>(NumBytes);
    static constexpr size_t NumElements       = SizeActual / Alignment;

    WordT                   data[NumElements] = {};

    class bit_reference {
      bitset_t &parent_;
      size_t    index_;

    public:
      bit_reference(bitset_t &parent, const size_t index)
          : parent_(parent), index_(index) {}

      operator bool() const {  // Implicit
        return parent_.get(index_);
      }

      bit_reference &operator=(const bool value) {
        parent_.set(index_, value);
        return *this;
      }

      bit_reference &operator=(const bit_reference &other) {
        return *this = static_cast<bool>(other);
      }

      bit_reference &operator&=(bool value) {
        const bool current = parent_.get(index_);
        parent_.set(index_, current & value);
        return *this;
      }

      // Bitwise OR assignment
      bit_reference &operator|=(bool value) {
        const bool current = parent_.get(index_);
        parent_.set(index_, current | value);
        return *this;
      }

      // Bitwise XOR assignment (toggle)
      bit_reference &operator^=(bool value) {
        const bool current = parent_.get(index_);
        parent_.set(index_, current ^ value);
        return *this;
      }
    };

  public:
    // Constructor

    constexpr bitset_t()                      = default;

    constexpr bitset_t(const bitset_t &other) = default;

    constexpr bitset_t(bitset_t &&other)      = default;

    // Methods

    size_t find_first_true() {
      for (size_t i = 0; i < NumElements; ++i) {
        if (!data[i])
          continue;
        for (size_t j = 0; j < 8 * Alignment; ++j) {
          if ((data[i] >> j) & 1)
            return i * Alignment + j;
        }
      }
      return size();
    }

    size_t find_first_false() {
      for (size_t i = 0; i < NumElements; ++i) {
        if (!~data[i])
          continue;
        for (size_t j = 0; j < 8 * Alignment; ++j) {
          if (!((data[i] >> j) & 1))
            return i * Alignment + j;
        }
      }
      return size();
    }

    [[nodiscard]] constexpr bool get(const size_t index) const {
      size_t idx_word = index / (8 * Alignment);
      size_t idx_bit  = index % (8 * Alignment);
      return (data[idx_word] >> idx_bit) & 1;
    }

    constexpr void set(const size_t index, bool value) {
      size_t idx_word = index / (8 * Alignment);
      size_t idx_bit  = index % (8 * Alignment);

      data[idx_word] &= ~(static_cast<WordT>(1) << idx_bit);
      data[idx_word] |= static_cast<WordT>(value) << idx_bit;
    }

    bit_reference operator[](const size_t index) {
      return bit_reference(*this, index);
    }


    [[nodiscard]] constexpr bool operator[](const size_t index) const {
      return this->get(index);
    }

    constexpr void clear(const size_t index) {
      size_t idx_word = index / (8 * Alignment);
      size_t idx_bit  = index % (8 * Alignment);

      data[idx_word] &= ~(static_cast<WordT>(1) << idx_bit);
    }

    constexpr void toggle(const size_t index) {
      size_t idx_word = index / (8 * Alignment);
      size_t idx_bit  = index % (8 * Alignment);

      data[idx_word] ^= static_cast<WordT>(1) << idx_bit;
    }

    void clear_all() {
      memset(data, 0x00, SizeActual);
    }

    void set_all() {
      memset(data, 0xFF, SizeActual);
    }

    bitset_t &operator&=(const bitset_t &other) {
      for (size_t i = 0; i < NumElements; ++i) {
        data[i] &= other.data[i];
      }
      return *this;
    }

    bitset_t &operator|=(const bitset_t &other) {
      for (size_t i = 0; i < NumElements; ++i) {
        data[i] |= other.data[i];
      }
      return *this;
    }

    bitset_t operator&(const bitset_t &other) const {
      bitset_t dst{*this};
      dst &= other;
      return dst;
    }

    bitset_t operator|(const bitset_t &other) const {
      bitset_t dst{*this};
      dst |= other;
      return dst;
    }

    bool operator==(const bitset_t &other) const {
      for (size_t i = 0; i < NumElements; ++i) {
        if (data[i] != other.data[i]) return false;
      }
      return true;
    }

    bool operator!=(const bitset_t &other) const {
      return !(this->operator==(other));
    }

    // Capacity

    [[nodiscard]] constexpr size_t size() const {
      return Nb;
    }

    [[nodiscard]] constexpr size_t capacity() const {
      return Nb;
    }
  };
}  // namespace xcore::container

namespace xcore {
  using namespace container;
}

#endif  //LIB_XCORE_CONTAINER_BITSET_HPP
