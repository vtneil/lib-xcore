#ifndef DEQUE_HPP
#define DEQUE_HPP

#include "container/array.hpp"

namespace container {
  template<typename Tp, size_t Capacity, template<typename, size_t> class Array = array_t>
  struct deque_t {
  protected:
    using value_type               = Tp;
    using reference                = value_type &;
    using const_reference          = const value_type &;

    Array<Tp, Capacity> arr_       = {};
    size_t              pos_front_ = {};
    size_t              pos_back_  = {};
    size_t              size_      = 0;

  public:
    // Deque Modification
    bool push_back(const Tp &t) {
      if (full())
        return false;

      arr_[pos_back_] = t;
      pos_back_       = utils::cyclic<Capacity>(pos_back_ + 1);
      ++size_;
      return true;
    }

    bool push_back(Tp &&t) {
      if (full())
        return false;

      arr_[pos_back_] = ported::move(t);
      pos_back_       = utils::cyclic<Capacity>(pos_back_ + 1);
      ++size_;
      return true;
    }

    template<typename... Args>
    bool emplace_back(Args &&...args) {
      static_assert(ported::is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");
      return push_back(ported::forward<Tp>(Tp(ported::forward<Args>(args)...)));
    }

    bool push_front(const Tp &t) {
      if (full())
        return false;

      pos_front_       = utils::cyclic<Capacity>(pos_front_ - 1);
      arr_[pos_front_] = t;
      ++size_;
      return true;
    }

    bool push_front(Tp &&t) {
      if (full())
        return false;

      pos_front_       = utils::cyclic<Capacity>(pos_front_ - 1);
      arr_[pos_front_] = ported::move(t);
      ++size_;
      return true;
    }

    template<typename... Args>
    bool emplace_front(Args &&...args) {
      static_assert(ported::is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");
      return push_front(ported::forward<Tp>(Tp(ported::forward<Args>(args)...)));
    }

    ported::optional<Tp> pop_front() {
      if (empty())
        return ported::nullopt;

      const size_t saved_id = pos_front_;
      pos_front_            = utils::cyclic<Capacity>(pos_front_ + 1);
      --size_;
      return arr_[saved_id];
    }

    ported::optional<Tp> pop_back() {
      if (empty())
        return ported::nullopt;

      pos_back_ = utils::cyclic<Capacity>(pos_back_ - 1);
      --size_;
      return arr_[pos_back_];
    }

    // Extended element access
    FORCE_INLINE constexpr reference front() noexcept {
      return arr_[pos_front_];
    }

    FORCE_INLINE constexpr const_reference front() const noexcept {
      return arr_[pos_front_];
    }

    FORCE_INLINE constexpr reference back() noexcept {
      return arr_[utils::cyclic<Capacity>(pos_back_ - 1)];
    }

    FORCE_INLINE constexpr const_reference back() const noexcept {
      return arr_[utils::cyclic<Capacity>(pos_back_ - 1)];
    }

    // Capacity
    [[nodiscard]] FORCE_INLINE constexpr size_t size() const noexcept { return size_; }

    [[nodiscard]] FORCE_INLINE constexpr size_t capacity() const noexcept { return Capacity; }

    [[nodiscard]] FORCE_INLINE constexpr bool   available_for(const size_t n) const { return Capacity - size() >= n; }

    [[nodiscard]] FORCE_INLINE constexpr bool   empty() const { return size() == 0; }

    [[nodiscard]] FORCE_INLINE constexpr bool   full() const { return !available_for(1); }

    constexpr const Tp                         *data() const { return arr_; }
  };
}  // namespace container

#endif  //DEQUE_HPP
