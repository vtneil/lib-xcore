#ifndef LIB_XCORE_CORE_PORTED_TUPLE_HPP
#define LIB_XCORE_CORE_PORTED_TUPLE_HPP

#include "internal/macros.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<typename... Ts>
struct tuple;

template<>
struct tuple<> {};

namespace detail {
  template<size_t, typename>
  struct tuple_element;

  template<size_t I, typename T, typename... Ts>
  struct tuple_element<I, tuple<T, Ts...>> : tuple_element<I - 1, tuple<Ts...>> {};

  template<typename T, typename... Ts>
  struct tuple_element<0, tuple<T, Ts...>> {
    using type = T;
  };
}  // namespace detail

template<typename T, typename... Ts>
struct tuple<T, Ts...> : private tuple<Ts...> {
private:
  T value;

public:
  constexpr tuple()                  = default;
  constexpr tuple(const tuple &)     = default;
  constexpr tuple(tuple &&) noexcept = default;
  explicit constexpr tuple(T value, Ts... values)
      : tuple<Ts...>(values...), value{value} {}

  constexpr tuple &operator=(const tuple &other) {
    value                              = other.value;
    static_cast<tuple<Ts...> &>(*this) = static_cast<const tuple<Ts...> &>(other);
    return *this;
  }

  constexpr tuple &operator=(tuple &&other) noexcept {
    value                              = move(other.value);
    static_cast<tuple<Ts...> &>(*this) = move(static_cast<const tuple<Ts...> &>(other));
    return *this;
  }

  template<size_t I, typename U, typename... Us>
  friend constexpr auto get(tuple<U, Us...> &t) -> enable_if_t<I == 0, U &>;

  template<size_t I, typename U, typename... Us>
  friend constexpr auto get(tuple<U, Us...> &t) -> enable_if_t<I != 0, typename detail::tuple_element<I, tuple<U, Us...>>::type &>;
};

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &t) -> enable_if_t<I == 0, T &> {
  return t.value;
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &t) -> enable_if_t<I != 0, typename detail::tuple_element<I, tuple<T, Ts...>>::type &> {
  return get<I - 1>(static_cast<tuple<Ts...> &>(t));
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(const tuple<T, Ts...> &t) -> enable_if_t<I == 0, const T &> {
  return t.value;
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(const tuple<T, Ts...> &t) -> enable_if_t<I != 0, const typename detail::tuple_element<I, tuple<T, Ts...>>::type &> {
  return get<I - 1>(static_cast<const tuple<Ts...> &>(t));
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &&t) -> enable_if_t<I == 0, T &&> {
  return move(t.value);
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &&t) -> enable_if_t<I != 0, typename detail::tuple_element<I, tuple<T, Ts...>>::type &&> {
  return get<I - 1>(move(static_cast<tuple<Ts...> &&>(t)));
}

template<typename... Ts>
tuple<Ts...> make_tuple(Ts &&...ts) {
  return tuple<Ts...>(forward<Ts>(ts)...);
}

template<typename... Ts>
constexpr tuple<Ts &...> tie(Ts &...ts) noexcept {
  return tuple<Ts &...>(ts...);
}

LIB_XCORE_END_NAMESPACE

namespace std {
  template<typename T>
  class tuple_size;

  template<size_t I, typename T>
  class tuple_element;

  template<typename... Ts>
  struct tuple_size<LIB_XCORE_NAMESPACE::tuple<Ts...>>
      : xcore::integral_constant<size_t, sizeof...(Ts)> {};

  template<size_t I, typename... Ts>
  struct tuple_element<I, LIB_XCORE_NAMESPACE::tuple<Ts...>> {
    using type = typename LIB_XCORE_NAMESPACE::detail::tuple_element<I, LIB_XCORE_NAMESPACE::tuple<Ts...>>::type;
  };

}  // namespace std


#endif  //LIB_XCORE_CORE_PORTED_TUPLE_HPP
