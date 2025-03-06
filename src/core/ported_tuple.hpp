#ifndef LIB_XCORE_CORE_PORTED_TUPLE_HPP
#define LIB_XCORE_CORE_PORTED_TUPLE_HPP

#include "internal/macros.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<typename... Ts>
struct tuple;

template<>
struct tuple<> {};

template<size_t, typename>
struct tuple_element;

template<size_t I, typename T, typename... Ts>
struct tuple_element<I, tuple<T, Ts...>> : tuple_element<I - 1, tuple<Ts...>> {};

template<typename T, typename... Ts>
struct tuple_element<0, tuple<T, Ts...>> {
  using type = T;
};

template<size_t I, typename T>
struct tuple_element<I, const T> {
  using type = typename tuple_element<I, T>::type;
};

template<size_t I, typename T>
struct tuple_element<I, volatile T> {
  using type = typename tuple_element<I, T>::type;
};

template<size_t I, typename T>
struct tuple_element<I, const volatile T> {
  using type = typename tuple_element<I, T>::type;
};

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
  friend constexpr auto get(tuple<U, Us...> &t) -> enable_if_t<I != 0, typename tuple_element<I, tuple<U, Us...>>::type &>;

  template<size_t I, typename U, typename... Us>
  friend constexpr auto get(const tuple<U, Us...> &t) -> enable_if_t<I == 0, const U &>;

  template<size_t I, typename U, typename... Us>
  friend constexpr auto get(const tuple<U, Us...> &t) -> enable_if_t<I != 0, const typename tuple_element<I, tuple<U, Us...>>::type &>;

  template<size_t I, typename U, typename... Us>
  friend constexpr auto get(tuple<U, Us...> &&t) -> enable_if_t<I == 0, U &&>;

  template<size_t I, typename U, typename... Us>
  friend constexpr auto get(tuple<U, Us...> &&t) -> enable_if_t<I != 0, typename tuple_element<I, tuple<U, Us...>>::type &&>;
};

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &t) -> enable_if_t<I == 0, T &> {
  return t.value;
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &t) -> enable_if_t<I != 0, typename tuple_element<I, tuple<T, Ts...>>::type &> {
  return get<I - 1>(static_cast<tuple<Ts...> &>(t));
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(const tuple<T, Ts...> &t) -> enable_if_t<I == 0, const T &> {
  return t.value;
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(const tuple<T, Ts...> &t) -> enable_if_t<I != 0, const typename tuple_element<I, tuple<T, Ts...>>::type &> {
  return get<I - 1>(static_cast<const tuple<Ts...> &>(t));
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &&t) -> enable_if_t<I == 0, T &&> {
  return move(t.value);
}

template<size_t I, typename T, typename... Ts>
constexpr auto get(tuple<T, Ts...> &&t) -> enable_if_t<I != 0, typename tuple_element<I, tuple<T, Ts...>>::type &&> {
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

namespace detail {
  template<typename... Ts>
  struct type_tuple_cat;

  template<typename... Ls, typename... Rs>
  struct type_tuple_cat<tuple<Ls...>, tuple<Rs...>> {
    using type = tuple<Ls..., Rs...>;
  };

  template<typename Tuple1, typename Tuple2>
  using tuple_cat_t = typename type_tuple_cat<Tuple1, Tuple2>::type;

  template<typename Tuple1, typename Tuple2, size_t... I1, size_t... I2>
  constexpr auto tuple_cat_impl(Tuple1 &&t1, Tuple2 &&t2,
                                index_sequence<I1...>, index_sequence<I2...>) {
    return tuple<typename tuple_element<I1, remove_reference_t<Tuple1>>::type...,
                 typename tuple_element<I2, remove_reference_t<Tuple2>>::type...>(get<I1>(forward<Tuple1>(t1))...,
                                                                                  get<I2>(forward<Tuple2>(t2))...);
  }

  template<typename... Ls, typename... Rs>
  constexpr auto tuple_cat(const tuple<Ls...> &t1, const tuple<Rs...> &t2) {
    return detail::tuple_cat_impl(t1, t2,
                                  make_index_sequence<sizeof...(Ls)>{},
                                  make_index_sequence<sizeof...(Rs)>{});
  }

  template<typename... Ls, typename... Rs>
  constexpr auto tuple_cat(tuple<Ls...> &&t1, tuple<Rs...> &&t2) {
    return detail::tuple_cat_impl(forward<tuple<Ls...>>(t1), forward<tuple<Rs...>>(t2),
                                  make_index_sequence<sizeof...(Ls)>{},
                                  make_index_sequence<sizeof...(Rs)>{});
  }

  template<typename... Ls, typename... Rs>
  constexpr auto tuple_cat(const tuple<Ls...> &t1, tuple<Rs...> &&t2) {
    return detail::tuple_cat_impl(t1, forward<tuple<Rs...>>(t2),
                                  make_index_sequence<sizeof...(Ls)>{},
                                  make_index_sequence<sizeof...(Rs)>{});
  }

  template<typename... Ls, typename... Rs>
  constexpr auto tuple_cat(tuple<Ls...> &&t1, const tuple<Rs...> &t2) {
    return detail::tuple_cat_impl(forward<tuple<Ls...>>(t1), t2,
                                  make_index_sequence<sizeof...(Ls)>{},
                                  make_index_sequence<sizeof...(Rs)>{});
  }
}  // namespace detail

// Concatenates tuples

template<typename Tuple>
constexpr auto tuple_cat(Tuple &&t) {
  return forward<Tuple>(t);
}

template<typename First, typename Second, typename... Rest>
constexpr auto tuple_cat(First &&first, Second &&second, Rest &&...rest) {
  return tuple_cat(detail::tuple_cat(forward<First>(first), forward<Second>(second)), forward<Rest>(rest)...);
}

LIB_XCORE_END_NAMESPACE

// For C++ structured bindings
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
    using type = typename LIB_XCORE_NAMESPACE::tuple_element<I, LIB_XCORE_NAMESPACE::tuple<Ts...>>::type;
  };

}  // namespace std

// Custom tuple factory to generate special tuples

LIB_XCORE_BEGIN_NAMESPACE

namespace detail {
  template<size_t, typename>
  struct _repeated_tuple;

  template<size_t I, typename T>
  struct _repeated_tuple {
    static_assert(I != 0);
    using type = tuple_cat_t<tuple<T>, typename _repeated_tuple<I - 1, T>::type>;
  };

  template<typename T>
  struct _repeated_tuple<1, T> {
    using type = tuple<T>;
  };
}  // namespace detail

template<typename T, size_t N>
using repeated_tuple = typename detail::_repeated_tuple<N, T>::type;

LIB_XCORE_END_NAMESPACE
#endif  //LIB_XCORE_CORE_PORTED_TUPLE_HPP
