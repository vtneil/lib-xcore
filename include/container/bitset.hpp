#ifndef BITSET_HPP
#define BITSET_HPP

namespace container {
  template<size_t Nb, typename WordT = int>
  class bitset_t {
  private:
    using byte_t                        = int8_t;

    static constexpr size_t Alignment   = sizeof(WordT);
    static constexpr size_t NumBytes    = (Nb + 8 - 1) / 8;
    static constexpr size_t SizeActual  = memory::nearest_alignment<byte_t, Alignment>(NumBytes);
    static constexpr size_t NumElements = SizeActual / Alignment;

    size_t                  size_;
    size_t                  num_bytes_;
    size_t                  size_actual_;
    size_t                  num_elements_;
    WordT                   data[NumElements] = {};

  public:
    // Constructor

    constexpr bitset_t() : size_{Nb} {
      num_bytes_    = (size_ + 8 - 1) / 8;
      size_actual_  = memory::nearest_alignment<byte_t, Alignment>(num_bytes_);
      num_elements_ = size_actual_ / Alignment;
    }

    explicit constexpr bitset_t(const size_t N) : size_{N} {
      num_bytes_    = (size_ + 8 - 1) / 8;
      size_actual_  = memory::nearest_alignment<byte_t, Alignment>(num_bytes_);
      num_elements_ = size_actual_ / Alignment;
    }

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
      return size_;
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
      return size_;
    }

    [[nodiscard]] constexpr bool get(const size_t index) const {
      size_t idx_word = index / (8 * Alignment);
      size_t idx_bit  = index % (8 * Alignment);
      return (data[idx_word] >> idx_bit) & 1;
    }

    [[nodiscard]] constexpr bool operator[](const size_t index) const {
      return this->get(index);
    }

    constexpr void set(const size_t index, bool value) {
      size_t idx_word = index / (8 * Alignment);
      size_t idx_bit  = index % (8 * Alignment);

      data[idx_word] &= ~(static_cast<WordT>(1) << idx_bit);
      data[idx_word] |= static_cast<WordT>(value) << idx_bit;
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
      return size_;
    }

    [[nodiscard]] static constexpr size_t capacity() {
      return Nb;
    }
  };
}  // namespace container

#endif  //BITSET_HPP
