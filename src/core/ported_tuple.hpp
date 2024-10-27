#ifndef PORTED_TUPLE_HPP
#define PORTED_TUPLE_HPP

namespace ported {
  template<typename... Ts>
  struct tuple;

  template<>
  struct tuple<> {};

  template<typename T, typename... Ts>
  struct tuple<T, Ts...> : private tuple<Ts...> {
  private:
    T value;

  public:
    constexpr          tuple()                  = default;
    constexpr          tuple(const tuple &)     = default;
    constexpr          tuple(tuple &&) noexcept = default;
    explicit constexpr tuple(T value, Ts... values)
        : tuple<Ts...>(values...), value{value} {}

    constexpr tuple &operator=(const tuple &other) {
      value                              = other.value;
      static_cast<tuple<Ts...> &>(*this) = static_cast<const tuple<Ts...> &>(other);
      return *this;
    }

    constexpr tuple &operator=(tuple &&other) noexcept {
      value                              = ported::move(other.value);
      static_cast<tuple<Ts...> &>(*this) = ported::move(static_cast<const tuple<Ts...> &>(other));
      return *this;
    }

    template<size_t I, typename... Us>
    friend auto get(tuple<Us...> &t);
  };

  namespace detail {
    template<size_t, typename>
    struct tuple_element;

    template<size_t I, typename T, typename... Ts>
    struct tuple_element<I, tuple<T, Ts...>> : tuple_element<I - 1, tuple<Ts...>> {
    };

    template<typename T, typename... Ts>
    struct tuple_element<0, tuple<T, Ts...>> {
      using type = T;
    };
  }  // namespace detail

  template<size_t I, typename T, typename... Ts>
  auto get(tuple<T, Ts...> &t) -> enable_if_t<I == 0, T &> {
    return t.value;
  }

  template<size_t I, typename T, typename... Ts>
  auto get(tuple<T, Ts...> &t) -> enable_if_t<I != 0, typename detail::tuple_element<I, tuple<T, Ts...>>::type &> {
    return get<I - 1>(static_cast<tuple<Ts...> &>(t));
  }

  template<typename... Ts>
  tuple<Ts...> make_tuple(Ts &&...ts) {
    return tuple<Ts...>(ported::forward<Ts>(ts)...);
  }

  template<typename... Ts>
  constexpr tuple<Ts &...> tie(Ts &...ts) noexcept {
    return tuple<Ts &...>(ts...);
  }
}  // namespace ported

#endif  //PORTED_TUPLE_HPP
