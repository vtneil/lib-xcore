#ifndef LIB_XCORE_CONTAINER_STACK_HPP
#define LIB_XCORE_CONTAINER_STACK_HPP

#include "container/deque.hpp"

namespace xcore::container {
  template<typename Tp, size_t Capacity, template<typename, size_t> class Container = array_t>
  struct stack_t : protected deque_t<Tp, Capacity, Container> {
  protected:
    using Base = deque_t<Tp, Capacity, Container>;

  public:
    using Base::available_for;
    using Base::Base;
    using Base::capacity;
    using Base::data;
    using Base::empty;
    using Base::full;
    using Base::size;

    bool push(const Tp &value) {
      return this->push_back(value);
    }

    bool push(Tp &&value) {
      return this->push_back(move(value));
    }

    bool push_force(const Tp &value) {
      return this->push_back_force(value);
    }

    bool push_force(Tp &&value) {
      return this->push_back_force(move(value));
    }

    template<typename... Args>
    bool emplace(Args &&...args) {
      return this->emplace_back(forward<Args>(args)...);
    }

    template<typename... Args>
    bool emplace_force(Args &&...args) {
      return this->emplace_back_force(forward<Args>(args)...);
    }

    optional<Tp> pop() {
      return this->pop_back();
    }

    [[nodiscard]] optional<Tp> peek() const {
      return this->back();
    }
  };
}  // namespace container

namespace xcore {
  using namespace container;
}

#endif  //LIB_XCORE_CONTAINER_STACK_HPP
