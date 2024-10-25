#ifndef STACK_HPP
#define STACK_HPP

#include "container/deque.hpp"

namespace container {
  template<typename Tp, size_t Capacity, template<typename, size_t> class Array = array_t>
  struct stack_t : protected deque_t<Tp, Capacity, Array> {
    using Base = deque_t<Tp, Capacity, Array>;

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

    template<typename... Args>
    bool emplace(Args &&...args) {
      return Base::emplace_back(ported::forward<Args>(args)...);
    }

    ported::optional<Tp> pop() {
      return Base::pop_back();
    }
  };
}  // namespace container

#endif  //STACK_HPP