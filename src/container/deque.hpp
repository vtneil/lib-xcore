#ifndef LIB_XCORE_CONTAINER_DEQUE_HPP
#define LIB_XCORE_CONTAINER_DEQUE_HPP

#include "internal/macros.hpp"
#include "container/array.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace container {
  template<typename Tp, size_t Capacity, template<typename, size_t> class Container = array_t>
  struct deque_t {
  protected:
    Container<Tp, Capacity> arr_       = {};
    size_t                  pos_front_ = {};
    size_t                  pos_back_  = {};
    size_t                  size_      = 0;

  public:
    // Deque Modification
    bool push_back(const Tp &t) {
      if (full())
        return false;
      _internal_push_back(t);
      return true;
    }

    bool push_back(Tp &&t) {
      if (full())
        return false;
      _internal_push_back(forward<Tp>(t));
      return true;
    }

    bool push_back_force(const Tp &t) {
      if (full()) {
        pos_front_ = utils::cyclic<Capacity>(pos_front_ + 1);
        --size_;
      }
      _internal_push_back(t);
      return true;
    }

    bool push_back_force(Tp &&t) {
      if (full()) {
        pos_front_ = utils::cyclic<Capacity>(pos_front_ + 1);
        --size_;
      }
      _internal_push_back(forward<Tp>(t));
      return true;
    }

    template<typename... Args>
    bool emplace_back(Args &&...args) {
      static_assert(is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");
      return push_back(Tp(forward<Args>(args)...));
    }

    template<typename... Args>
    bool emplace_back_force(Args &&...args) {
      static_assert(is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");
      return push_back_force(Tp(forward<Args>(args)...));
    }

    bool push_front(const Tp &t) {
      if (full())
        return false;
      _internal_push_front(t);
      return true;
    }

    bool push_front(Tp &&t) {
      if (full())
        return false;
      _internal_push_front(forward<Tp>(t));
      return true;
    }

    bool push_front_force(const Tp &t) {
      if (full()) {
        pos_back_ = utils::cyclic<Capacity>(pos_back_ - 1);
        --size_;
      }
      _internal_push_front(t);
      return true;
    }

    bool push_front_force(Tp &&t) {
      if (full()) {
        pos_back_ = utils::cyclic<Capacity>(pos_back_ - 1);
        --size_;
      }
      _internal_push_front(forward<Tp>(t));
      return true;
    }

    template<typename... Args>
    bool emplace_front(Args &&...args) {
      static_assert(is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");
      return push_front(Tp(forward<Args>(args)...));
    }

    template<typename... Args>
    bool emplace_front_force(Args &&...args) {
      static_assert(is_constructible_v<Tp, Args &&...>, "Arguments cannot construct the type");
      return push_front_force(Tp(forward<Args>(args)...));
    }

    optional<Tp> pop_front() {
      if (empty())
        return nullopt;

      const size_t saved_id = pos_front_;
      pos_front_            = utils::cyclic<Capacity>(pos_front_ + 1);
      --size_;
      return arr_[saved_id];
    }

    optional<Tp> pop_back() {
      if (empty())
        return nullopt;

      pos_back_ = utils::cyclic<Capacity>(pos_back_ - 1);
      --size_;
      return arr_[pos_back_];
    }

    // Extended element access with optional support (always copy, no referenced access)

    FORCE_INLINE constexpr optional<Tp> front() const noexcept {
      if (empty())
        return nullopt;
      return arr_[pos_front_];
    }

    FORCE_INLINE constexpr optional<Tp> back() const noexcept {
      if (empty())
        return nullopt;
      return arr_[utils::cyclic<Capacity>(pos_back_ - 1)];
    }

    // Capacity
    [[nodiscard]] FORCE_INLINE constexpr size_t size() const noexcept { return size_; }

    [[nodiscard]] FORCE_INLINE constexpr size_t capacity() const noexcept { return Capacity; }

    [[nodiscard]] FORCE_INLINE constexpr bool   available_for(const size_t n) const { return (n <= Capacity) && (Capacity - size() >= n); }

    [[nodiscard]] FORCE_INLINE constexpr bool   empty() const { return size() == 0; }

    [[nodiscard]] FORCE_INLINE constexpr bool   full() const { return !available_for(1); }

    constexpr const Tp                         *data() const { return arr_; }

  protected:
    FORCE_INLINE void _internal_push_back(const Tp &t) {
      arr_[pos_back_] = t;
      pos_back_       = utils::cyclic<Capacity>(pos_back_ + 1);
      ++size_;
    }

    FORCE_INLINE void _internal_push_back(Tp &&t) {
      arr_[pos_back_] = move(t);
      pos_back_       = utils::cyclic<Capacity>(pos_back_ + 1);
      ++size_;
    }

    FORCE_INLINE void _internal_push_front(const Tp &t) {
      pos_front_       = utils::cyclic<Capacity>(pos_front_ - 1);
      arr_[pos_front_] = t;
      ++size_;
    }

    FORCE_INLINE void _internal_push_front(Tp &&t) {
      pos_front_       = utils::cyclic<Capacity>(pos_front_ - 1);
      arr_[pos_front_] = move(t);
      ++size_;
    }
  };
}  // namespace container

using namespace container;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CONTAINER_DEQUE_HPP
