#ifndef PORTED_REFWRAP_HPP
#define PORTED_REFWRAP_HPP

#include "core/ported_move.hpp"
#include "core/ported_type_traits.hpp"

namespace ported {
  // REFERENCE WRAPPER

  template<typename T>
  struct reference_wrapper {
    using type = T;

    explicit           reference_wrapper(T &ref) noexcept : _ref(&ref) {}

                       reference_wrapper(const reference_wrapper &other) noexcept = default;

    reference_wrapper &operator=(const reference_wrapper &other) noexcept         = default;

    T                 &get() const noexcept {
      return *_ref;
    }

    operator T &() const noexcept {
      return get();
    }

    template<typename... Args>
    decltype(auto) operator()(Args &&...args) const {
      return (*_ref)(forward<Args>(args)...);
    }

  private:
    T *_ref;
  };

  template<typename T>
  reference_wrapper<T> ref(T &t) noexcept {
    return reference_wrapper<T>(t);
  }

  template<typename T>
  reference_wrapper<const T> cref(const T &t) noexcept {
    return reference_wrapper<const T>(t);
  }
}  // namespace ported

#endif  //PORTED_REFWRAP_HPP
