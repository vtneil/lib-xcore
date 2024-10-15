#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include "core/ported_std.hpp"
#include "core/optional.hpp"

namespace container {
  namespace impl {
    template<size_t Capacity>
    constexpr size_t cyclic(const size_t current) { return current % Capacity; }
  }  // namespace impl

  template<typename Tp, size_t Capacity>
  class circular_buffer_static_t {
  public:
    static_assert(Capacity > 0, "Buffer capacity must not be 0.");

  private:
    Tp     arr_[Capacity] = {};
    size_t start_index_   = 0;
    size_t size_          = 0;

  public:
    circular_buffer_static_t() = default;

    circular_buffer_static_t(const circular_buffer_static_t &other) {
      start_index_ = other.start_index_;
      size_        = other.size_;
      ported::copy(other.arr_, other.arr_ + Capacity, arr_);
    }

    circular_buffer_static_t(circular_buffer_static_t &&other) noexcept {
      start_index_       = other.start_index_;
      size_              = other.size_;
      other.start_index_ = 0;
      other.size_        = 0;
      for (size_t i = 0; i < Capacity; ++i) arr_[i] = ported::move(other.arr_[i]);
    }

    bool put(const Tp &t) {
      if (full())
        return false;

      arr_[impl::cyclic<Capacity>(start_index_ + size_)] = t;
      static_cast<void>(++size_);
      return true;
    }

    bool put(Tp &&t) {
      if (full())
        return false;

      arr_[impl::cyclic<Capacity>(start_index_ + size_)] = ported::move(t);
      static_cast<void>(++size_);
      return true;
    }

    template<typename... Args>
    bool emplace(Args &&...args) {
      static_assert(ported::is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");

      if (full())
        return false;

      new (arr_ + impl::cyclic<Capacity>(start_index_ + size_)) Tp(ported::forward<Args>(args)...);
      static_cast<void>(++size_);
      return true;
    }

    ported::optional<Tp> get() {
      if (empty())
        return ported::nullopt;

      const size_t saved_id = start_index_;
      start_index_          = impl::cyclic<Capacity>(start_index_ + 1);
      static_cast<void>(--size_);
      return arr_[saved_id];
    }

    [[nodiscard]] constexpr bool   empty() const { return (size_ == 0); }

    [[nodiscard]] constexpr bool   full() const { return !available_for(1); }

    [[nodiscard]] constexpr bool   available_for(const size_t size) const { return (Capacity - size_ >= size); }

    [[nodiscard]] constexpr size_t size() const { return size_; }

    [[nodiscard]] constexpr size_t capacity() const { return Capacity; }

    constexpr const Tp            *data() const { return arr_; }
  };
}  // namespace container

#endif  //CIRCULAR_BUFFER_HPP
