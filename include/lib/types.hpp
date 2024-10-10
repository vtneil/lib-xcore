#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>

namespace traits {
  template<typename S, typename T>
  concept has_ostream = requires(S s, T t) {
    { s << t } -> std::same_as<S &>;
  };
}  // namespace traits

namespace types {

}

#endif  //TYPES_HPP
