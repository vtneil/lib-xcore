#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>

namespace traits {
  template<typename S>
  concept has_ostream = requires(S s, const char *str) {
    { s << str } -> std::same_as<S &>;
  };
}  // namespace traits

namespace types {

}

#endif  //TYPES_HPP
