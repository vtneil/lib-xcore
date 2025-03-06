#ifndef LIB_XCORE_CORE_PORTED_PORTED_VARIANT_HPP
#define LIB_XCORE_CORE_PORTED_PORTED_VARIANT_HPP

#include "internal/macros.hpp"

#include "core/ported_std.hpp"
#include "core/ported_type_traits.hpp"
#include "core/ported_tuple.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<size_t Len, class... Types>
struct aligned_union {
  static constexpr size_t alignment_value = max(alignof(Types)...);

  struct type {
    alignas(alignment_value) char _s[max(Len, sizeof(Types)...)];
  };
};

template<size_t Len, class... Types>
using aligned_union_t = typename aligned_union<Len, Types...>::type;

namespace detail {
  template<typename T, typename... Ts>
  struct type_index;

  template<typename T, typename First, typename... Rest>
  struct type_index<T, First, Rest...> {
    static constexpr size_t value = 1 + type_index<T, Rest...>::value;
  };

  template<typename T, typename... Rest>
  struct type_index<T, T, Rest...> {
    static constexpr size_t value = 0;
  };

  template<typename T, typename... Ts>
  struct contains_type;

  template<typename T>
  struct contains_type<T> : false_type {};

  template<typename T, typename First, typename... Rest>
  struct contains_type<T, First, Rest...> {
    static constexpr bool value = is_same_v<T, First> || contains_type<T, Rest...>::value;
  };

  // Base class for types in variant storage
  template<typename... Ts>
  union storage {
    void                     *dummy;
    aligned_union_t<0, Ts...> data;

    // Constructor
    storage() : dummy(nullptr) {}

    // Destructor: do not destroy
    ~storage() {}
  };

}  // namespace detail

template<typename... Types>
struct variant;

template<typename T, typename... Types>
constexpr bool holds_alternative(const variant<Types...> &) noexcept {
  return detail::contains_type<T, Types...>::value;
}

template<typename... Types>
struct variant {
private:
  static_assert(sizeof...(Types) > 0, "variant must have at least one alternative type");

  detail::storage<Types...> data_;
  size_t                    type_index_;

public:
  // Default constructor
  variant() : type_index_(0) {
    using FirstType = typename tuple_element<0, tuple<Types...>>::type;
    new (&data_) FirstType();
  }

  // Copy constructor
  variant(const variant &other) : type_index_(other.type_index_) {
    copy_construct(other);
  }

  // Move constructor
  variant(variant &&other) noexcept : type_index_(other.type_index_) {
    move_construct(move(other));
  }

  // Converting constructor
  template<typename T, typename = enable_if_t<detail::contains_type<T, Types...>::value>>
  explicit variant(T &&value) : type_index_(detail::type_index<T, Types...>::value) {
    new (&data_) T(forward<T>(value));
  }

  // Destructor
  ~variant() {
    destroy();
  }

  // Copy assignment operator
  variant &operator=(const variant &other) {
    if (this != &other) {
      destroy();
      type_index_ = other.type_index_;
      copy_construct(other);
    }
    return *this;
  }

  // Move assignment operator
  variant &operator=(variant &&other) noexcept {
    if (this != &other) {
      destroy();
      type_index_ = other.type_index_;
      move_construct(move(other));
    }
    return *this;
  }

  // Conversion assignment operator
  template<typename T, typename = enable_if_t<detail::contains_type<T, Types...>::value>>
  variant &operator=(T &&value) {
    destroy();
    type_index_ = detail::type_index<T, Types...>::value;
    new (&data_) T(forward<T>(value));
    return *this;
  }

  // Emplace
  template<typename T, typename... Args>
  T &emplace(Args &&...args) {
    static_assert(detail::contains_type<T, Types...>::value, "Type not in variant");
    destroy();
    type_index_ = detail::type_index<T, Types...>::value;
    new (&data_) T(forward<Args>(args)...);
    return *reinterpret_cast<T *>(&data_);
  }

  [[nodiscard]] size_t index() const noexcept {
    return type_index_;
  }

private:
  void copy_construct(const variant &other) {
    using copier = int[];
    (void) copier{0, (type_index_ == detail::type_index<Types, Types...>::value
                        ? (new (&data_) Types(*reinterpret_cast<const Types *>(&other.data_)), 0)
                        : 0)...};
  }

  void move_construct(variant &&other) {
    using mover = int[];
    (void) mover{0, (type_index_ == detail::type_index<Types, Types...>::value
                       ? (new (&data_) Types(move(*reinterpret_cast<Types *>(&other.data_))), 0)
                       : 0)...};
  }

  void destroy() {
    using destroyer = int[];
    (void) destroyer{0, (type_index_ == detail::type_index<Types, Types...>::value
                           ? (reinterpret_cast<Types *>(&data_)->~Types(), 0)
                           : 0)...};
  }

  template<typename T, typename... Ts>
  friend constexpr T &get(variant<Ts...> &var);

  template<typename T, typename... Ts>
  friend constexpr const T &get(const variant<Ts...> &var);

  template<size_t I, typename... Ts>
  friend constexpr auto &get(variant<Ts...> &var);

  template<size_t I, typename... Ts>
  friend constexpr const auto &get(const variant<Ts...> &var);
};

template<typename T, typename... Types>
constexpr T &get(variant<Types...> &var) {
  static_assert(detail::contains_type<T, Types...>::value, "Type not in variant");
  return *reinterpret_cast<T *>(&var.data_);
}

template<typename T, typename... Types>
constexpr const T &get(const variant<Types...> &var) {
  static_assert(detail::contains_type<T, Types...>::value, "Type not in variant");
  return *reinterpret_cast<const T *>(&var.data_);
}

// Non-member `get<I>` function
template<size_t I, typename... Types>
constexpr auto &get(variant<Types...> &var) {
  using T = typename tuple_element<I, tuple<Types...>>::type;
  static_assert(I < sizeof...(Types), "Index out of bounds in variant");
  return *reinterpret_cast<T *>(&var.data_);
}

template<size_t I, typename... Types>
constexpr const auto &get(const variant<Types...> &var) {
  using T = typename tuple_element<I, tuple<Types...>>::type;
  static_assert(I < sizeof...(Types), "Index out of bounds in variant");
  return *reinterpret_cast<const T *>(&var.data_);
}

struct monostate {};

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_CORE_PORTED_PORTED_VARIANT_HPP
