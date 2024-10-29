#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "container/deque.hpp"

namespace container {
  template<typename Tp, size_t Capacity, template<typename, size_t> class Container = array_t>
  struct queue_t : protected deque_t<Tp, Capacity, Container> {
    using Base = deque_t<Tp, Capacity, Container>;

    using Base::available_for;
    using Base::Base;
    using Base::capacity;
    using Base::data;
    using Base::empty;
    using Base::front;
    using Base::full;
    using Base::size;

    bool push(const Tp &value) {
      return Base::push_back(value);
    }

    bool push(Tp &&value) {
      return Base::push_back(ported::move(value));
    }

    bool push_force(const Tp &value) {
      return Base::push_back_force(value);
    }

    bool push_force(Tp &&value) {
      return Base::push_back_force(ported::move(value));
    }

    template<typename... Args>
    bool emplace(Args &&...args) {
      return Base::emplace_back(ported::forward<Args>(args)...);
    }

    template<typename... Args>
    bool emplace_force(Args &&...args) {
      return Base::emplace_back_force(ported::forward<Args>(args)...);
    }

    ported::optional<Tp> pop() {
      return Base::pop_front();
    }
  };
}  // namespace container

#endif  //QUEUE_HPP
