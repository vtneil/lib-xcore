#ifndef LIB_XCORE_CONTAINER_BYTE_BUFFER_HPP
#define LIB_XCORE_CONTAINER_BYTE_BUFFER_HPP

#include "internal/macros.hpp"
#include "container/deque.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdint>

LIB_XCORE_BEGIN_NAMESPACE

namespace container {
  template<size_t Capacity, template<typename, size_t> class Container = array_t>
  struct byte_buffer_t : protected deque_t<unsigned char, Capacity, Container> {
  protected:
    using Base = deque_t<unsigned char, Capacity, Container>;

  public:
    using Base::available_for;
    using Base::Base;
    using Base::capacity;
    using Base::empty;
    using Base::full;
    using Base::size;

    bool push(const unsigned char byte) { return push(&byte, 1); }

    bool push(const unsigned char *src, const size_t n) {
      if (n == 0)
        return true;

      if (!this->available_for(n))
        return false;

      _internal_push(src, n);
      return true;
    }

    bool push_force(const unsigned char byte) { return push_force(&byte, 1); }

    bool push_force(const unsigned char *src, const size_t n) {
      if (n == 0)
        return true;

      if (n > Capacity)
        return false;  // Always reject

      if (const size_t space_left = Capacity - this->size();
          n > space_left) {
        const size_t overflow = n - space_left;
        this->pos_front_      = utils::cyclic<Capacity>(this->pos_front_ + overflow);
        this->size_ -= overflow;
      }

      _internal_push(src, n);
      return true;
    }

    optional<unsigned char *> peek(unsigned char *dst, const size_t n) const {
      if (n == 0)
        return dst;

      if (n > this->size_)
        return nullopt;

      const size_t pos_to_read = this->pos_front_;

      if (pos_to_read + n <= Capacity) {
        // No wraparound
        memcpy(dst, this->arr_ + pos_to_read, n);
      } else {
        // Wraparound
        const size_t first_part_len = Capacity - pos_to_read;
        memcpy(dst, this->arr_ + pos_to_read, first_part_len);
        memcpy(dst + first_part_len, this->arr_, n - first_part_len);
      }

      return dst;
    }

    optional<unsigned char *> pop(unsigned char *dst, const size_t n) {
      const auto ret_opt = peek(dst, n);
      if (!ret_opt)
        return nullopt;
      this->pos_front_ = utils::cyclic<Capacity>(this->pos_front_ + n);
      this->size_ -= n;
      return ret_opt;
    }

  protected:
    void _internal_push(const unsigned char *src, const size_t n) {
      const size_t pos_to_insert = this->pos_back_;

      if (pos_to_insert + n <= Capacity) {
        // No wraparound
        memcpy(this->arr_ + pos_to_insert, src, n);
      } else {
        // Wraparound
        const size_t first_part_len = Capacity - pos_to_insert;
        memcpy(this->arr_ + pos_to_insert, src, first_part_len);
        memcpy(this->arr_, src + first_part_len, n - first_part_len);
      }

      this->pos_back_ = utils::cyclic<Capacity>(this->pos_back_ + n);
      this->size_ += n;
    }
  };
}  // namespace container

using namespace container;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CONTAINER_BYTE_BUFFER_HPP
