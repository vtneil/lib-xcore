#ifndef LIB_XCORE_CORE_PORTED_TYPE_TRAITS_HPP
#define LIB_XCORE_CORE_PORTED_TYPE_TRAITS_HPP

#include "core/macros_bootstrap.hpp"
#include "core/ported_config.hpp"

namespace xcore {
  template<typename...>
  using void_t = void;

  template<typename T>
  struct remove_reference {
    using type = T;
  };

  template<typename T>
  struct remove_reference<T &> {
    using type = T;
  };

  template<typename T>
  struct remove_reference<T &&> {
    using type = T;
  };

  template<typename T>
  using remove_reference_t = typename remove_reference<T>::type;

  template<class T>
  struct remove_const {
    typedef T type;
  };
  template<class T>
  struct remove_const<const T> {
    typedef T type;
  };

  template<typename T>
  using remove_const_t = typename remove_const<T>::type;

  template<typename T>
  struct remove_cv {
    using type = T;
  };

  template<typename T>
  struct remove_cv<const T> {
    using type = T;
  };

  template<typename T>
  struct remove_cv<volatile T> {
    using type = T;
  };

  template<typename T>
  struct remove_cv<const volatile T> {
    using type = T;
  };

  template<typename T>
  using remove_cv_t = typename remove_cv<T>::type;

  // INTEGRAL_CONSTANT

  template<typename T, T v>
  struct integral_constant {
    static constexpr T        value = v;
    typedef T                 value_type;
    typedef integral_constant type;

    explicit constexpr        operator value_type() const noexcept { return value; }
  };

  template<bool v>
  using bool_constant = integral_constant<bool, v>;

  using true_type     = bool_constant<true>;

  using false_type    = bool_constant<false>;

  // CONDITIONALS

  template<bool, typename T, typename>
  struct conditional {
    using type = T;
  };

  template<typename T, typename F>
  struct conditional<false, T, F> {
    using type = F;
  };

  template<bool B, typename T, typename F>
  using conditional_t = typename conditional<B, T, F>::type;

  template<typename...>
  struct disjunction : false_type {};

  template<typename B1>
  struct disjunction<B1> : B1 {};

  template<typename B1, typename... Bn>
  struct disjunction<B1, Bn...>
      : conditional_t<static_cast<bool>(B1::value), B1, disjunction<Bn...>> {};

  template<typename...>
  struct conjunction : true_type {};

  template<typename B1>
  struct conjunction<B1> : B1 {};

  template<typename B1, typename... Bn>
  struct conjunction<B1, Bn...>
      : conditional_t<static_cast<bool>(B1::value), conjunction<Bn...>, B1> {};

  namespace detail {
    template<typename... Bs>
    struct or_impl : disjunction<Bs...> {};

    template<typename... Bs>
    struct and_impl : conjunction<Bs...> {};

    template<typename... Bs>
    inline constexpr bool or_impl_v = or_impl<Bs...>::value;

    template<typename... Bs>
    inline constexpr bool and_impl_v = and_impl<Bs...>::value;
  }  // namespace detail

  // IS_SAME

  template<typename, typename>
  struct is_same : false_type {};

  template<typename T>
  struct is_same<T, T> : true_type {};

  template<typename T, typename U>
  inline constexpr bool is_same_v = is_same<T, U>::value;

  // REFERENCES

  namespace detail {
    template<typename>
    struct always_false : false_type {};

    template<typename T>
    struct type_identity {
      using type = T;
    };

    template<typename T>
    auto try_add_lvalue_reference(int) -> type_identity<T &>;

    template<typename T>
    auto try_add_lvalue_reference(...) -> type_identity<T>;

    template<typename T>
    auto try_add_rvalue_reference(int) -> type_identity<T &&>;

    template<typename T>
    auto try_add_rvalue_reference(...) -> type_identity<T>;
  }  // namespace detail

  template<typename T>
  struct add_lvalue_reference : decltype(detail::try_add_lvalue_reference<T>(0)) {};

  template<typename T>
  struct add_rvalue_reference : decltype(detail::try_add_rvalue_reference<T>(0)) {};

  template<typename T>
  typename add_rvalue_reference<T>::type declval() noexcept {
    static_assert(detail::always_false<T>::value, "declval is not allowed in an evaluated context.");
  }

  // IS_REFERENCE

  template<typename>
  struct is_lvalue_reference : false_type {};

  template<typename T>
  struct is_lvalue_reference<T &> : true_type {};

  template<typename>
  struct is_rvalue_reference : false_type {};

  template<typename T>
  struct is_rvalue_reference<T &&> : true_type {};

  template<typename>
  struct is_reference : false_type {};

  template<typename T>
  struct is_reference<T &> : true_type {};

  template<typename T>
  struct is_reference<T &&> : true_type {};

  template<typename T>
  inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

  template<typename T>
  inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

  template<typename T>
  inline constexpr bool is_reference_v = is_reference<T>::value;

  // IS_TYPE

  template<typename>
  struct is_void : false_type {};

  template<>
  struct is_void<void> : true_type {};

  namespace detail {
    template<typename>
    struct is_integral_impl : false_type {};

    template<>
    struct is_integral_impl<bool> : true_type {};

    template<>
    struct is_integral_impl<char> : true_type {};

    template<>
    struct is_integral_impl<signed char> : true_type {};

    template<>
    struct is_integral_impl<unsigned char> : true_type {};

    template<>
    struct is_integral_impl<wchar_t> : true_type {};

    template<>
    struct is_integral_impl<char16_t> : true_type {};

    template<>
    struct is_integral_impl<char32_t> : true_type {};

    template<>
    struct is_integral_impl<short> : true_type {};

    template<>
    struct is_integral_impl<unsigned short> : true_type {};

    template<>
    struct is_integral_impl<int> : true_type {};

    template<>
    struct is_integral_impl<unsigned int> : true_type {};

    template<>
    struct is_integral_impl<long> : true_type {};

    template<>
    struct is_integral_impl<unsigned long> : true_type {};

    template<>
    struct is_integral_impl<long long> : true_type {};

    template<>
    struct is_integral_impl<unsigned long long> : true_type {};
  }  // namespace detail

  template<typename T>
  struct is_integral : detail::is_integral_impl<remove_cv_t<T>>::type {};

  namespace detail {
    template<typename>
    struct is_floating_point_impl : false_type {};

    template<>
    struct is_floating_point_impl<float> : true_type {};

    template<>
    struct is_floating_point_impl<double> : true_type {};

    template<>
    struct is_floating_point_impl<long double> : true_type {};
  }  // namespace detail

  template<typename T>
  struct is_floating_point : detail::is_floating_point_impl<remove_cv_t<T>>::type {};

  namespace detail {
    template<typename T>
    struct is_arithmetic_impl : or_impl<is_integral<T>, is_floating_point<T>> {};
  }  // namespace detail

  template<typename T>
  struct is_arithmetic : detail::is_arithmetic_impl<remove_cv_t<T>>::type {};

  namespace detail {
    template<typename>
    struct is_signed_impl : false_type {};

    template<>
    struct is_signed_impl<char> : bool_constant<(static_cast<char>(-1) < static_cast<char>(0))> {};

    template<>
    struct is_signed_impl<signed char> : true_type {};

    template<>
    struct is_signed_impl<short> : true_type {};

    template<>
    struct is_signed_impl<int> : true_type {};

    template<>
    struct is_signed_impl<long> : true_type {};

    template<>
    struct is_signed_impl<long long> : true_type {};
  }  // namespace detail

  template<typename T>
  struct is_signed : detail::is_signed_impl<remove_cv_t<T>>::type {};

  namespace detail {
    template<typename T, bool = is_arithmetic<T>::value>
    struct is_unsigned_impl : integral_constant<bool, T(0) < T(-1)> {};

    template<typename T>
    struct is_unsigned_impl<T, false> : false_type {};
  }  // namespace detail

  template<typename T>
  struct is_unsigned : detail::is_unsigned_impl<remove_cv_t<T>>::type {};

  namespace detail {
    template<typename T>
    struct make_unsigned_impl;

    template<>
    struct make_unsigned_impl<char> {
      using type = unsigned char;
    };

    template<>
    struct make_unsigned_impl<signed char> {
      using type = unsigned char;
    };

    template<>
    struct make_unsigned_impl<short> {
      using type = unsigned short;
    };

    template<>
    struct make_unsigned_impl<int> {
      using type = unsigned int;
    };

    template<>
    struct make_unsigned_impl<long> {
      using type = unsigned long;
    };

    template<>
    struct make_unsigned_impl<long long> {
      using type = unsigned long long;
    };

    template<>
    struct make_unsigned_impl<unsigned char> {
      using type = unsigned char;
    };

    template<>
    struct make_unsigned_impl<unsigned short> {
      using type = unsigned short;
    };

    template<>
    struct make_unsigned_impl<unsigned int> {
      using type = unsigned int;
    };

    template<>
    struct make_unsigned_impl<unsigned long> {
      using type = unsigned long;
    };

    template<>
    struct make_unsigned_impl<unsigned long long> {
      using type = unsigned long long;
    };
  }  // namespace detail

  template<typename T>
  struct make_unsigned : detail::make_unsigned_impl<remove_cv_t<T>> {};

  template<typename T>
  using make_unsigned_t = typename make_unsigned<T>::type;

  namespace detail {
    template<typename>
    struct is_null_pointer_impl : false_type {};

    template<>
    struct is_null_pointer_impl<nullptr_t> : true_type {};
  }  // namespace detail

  template<typename T>
  struct is_null_pointer : detail::is_null_pointer_impl<remove_cv_t<T>>::type {};

  template<typename T>
  inline constexpr bool is_void_v = is_void<T>::value;

  template<typename T>
  inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

  template<typename T>
  inline constexpr bool is_integral_v = is_integral<T>::value;

  template<typename T>
  inline constexpr bool is_signed_v = is_signed<T>::value;

  template<typename T>
  inline constexpr bool is_unsigned_v = is_unsigned<T>::value;

  template<typename T>
  inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

  // ENABLE IF

  template<bool, typename = void>
  struct enable_if {};

  template<typename T>
  struct enable_if<true, T> {
    typedef T type;
  };

  template<bool B, typename T = void>
  using enable_if_t = typename enable_if<B, T>::type;

  // CONSTRUCTIBLE

  namespace detail {
    template<typename T, typename... Args>
    class is_constructible_impl {
      template<typename U, typename... UArgs,
               typename = decltype(U(declval<UArgs>()...))>
      static true_type test(int);

      template<typename, typename...>
      static false_type test(...);

    public:
      using type                  = decltype(test<T, Args...>(0));
      static constexpr bool value = type::value;
    };
  }  // namespace detail

  template<typename T, typename... Args>
  using is_constructible = typename detail::is_constructible_impl<T, Args...>::type;

  namespace detail {
    template<typename T, typename... Args>
    class is_trivially_constructible_impl {
      static constexpr bool trivially_constructible =
        is_constructible<T, Args...>::value && __is_trivially_constructible(T, Args...);

    public:
      using type                  = integral_constant<bool, trivially_constructible>;
      static constexpr bool value = type::value;
    };
  }  // namespace detail

  template<typename T, typename... Args>
  using is_trivially_constructible = typename detail::is_trivially_constructible_impl<T, Args...>::type;

  namespace detail {
    template<typename T, typename... Args>
    class is_nothrow_constructible_impl {
      static constexpr bool nothrow_constructible = noexcept(T(declval<Args>()...));

    public:
      using type                  = integral_constant<bool, nothrow_constructible>;
      static constexpr bool value = type::value;
    };

  }  // namespace detail

  template<typename T, typename... Args>
  using is_nothrow_constructible = typename detail::is_nothrow_constructible_impl<T, Args...>::type;

  template<typename T, typename... Args>
  inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

  template<typename T, typename... Args>
  inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T, Args...>::value;

  template<typename T, typename... Args>
  inline constexpr bool is_nothrow_constructible_v = is_nothrow_constructible<T, Args...>::value;

  // IS_FUNCTION

  template<typename>
  struct is_function : false_type {};

  template<typename Ret, typename... Args>
  struct is_function<Ret(Args...)> : true_type {};

  template<typename Ret, typename... Args>
  struct is_function<Ret(Args..., ...)> : true_type {};

  template<typename Ret, typename... Args>
  struct is_function<Ret(Args...) noexcept> : true_type {};

  template<typename Ret, typename... Args>
  struct is_function<Ret(Args..., ...) noexcept> : true_type {};

  template<typename T>
  inline constexpr bool is_function_v = is_function<T>::value;

  // IS_CLASS

  template<typename T>
  struct is_class {
  private:
    template<typename U>
    static bool test(int U::*) { return true; }

    template<typename>
    static bool test(...) { return false; }

  public:
    static constexpr bool value = test<T>(nullptr);
  };

  template<typename T>
  inline constexpr bool is_class_v = is_class<T>::value;

  // IS_BASE_OF

  namespace detail {
    template<typename Base, typename Derived>
    struct is_base_of_impl {
    private:
      static constexpr bool test(Base *) { return true; }

      static constexpr bool test(...) { return false; }

    public:
      static constexpr bool value = test(static_cast<Derived *>(nullptr));
    };
  }  // namespace detail

  template<typename Base, typename Derived>
  struct is_base_of : bool_constant<
                        !is_same_v<Base, Derived> &&
                        is_class_v<Base> &&
                        is_class_v<Derived> &&
                        detail::is_base_of_impl<Base, Derived>::value> {};

  template<typename Base, typename Derived>
  inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

  // IS_OBJECT

  template<typename T>
  struct is_object
      : bool_constant<
          !is_function_v<T> &&
          !is_reference_v<T> &&
          !is_void_v<T>> {};

  template<typename T>
  inline constexpr bool is_object_v = is_object<T>::value;

  template<typename>
  struct is_member_pointer : false_type {};

  template<typename T, typename C>
  struct is_member_pointer<T C::*> : true_type {};

  template<typename T>
  inline constexpr bool is_member_pointer_v = is_member_pointer<T>::value;

  // IS_CONVERTIBLE

  namespace detail {
    template<typename, typename, typename = void>
    struct is_convertible_impl : false_type {};

    template<typename From, typename To>
    struct is_convertible_impl<From, To, void_t<decltype(static_cast<To>(declval<From>()))>> : true_type {};
  }  // namespace detail

  template<typename From, typename To>
  struct is_convertible : detail::is_convertible_impl<From, To> {};

  template<typename From, typename To>
  inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

  // DECAY
  namespace detail {
    template<typename T>
    struct decay_array {
      using type = T;
    };

    template<typename T, std::size_t N>
    struct decay_array<T[N]> {
      using type = T *;
    };

    template<typename T>
    struct decay_array<T[]> {
      using type = T *;
    };

    // Function to pointer transformation
    template<typename T>
    struct decay_function {
      using type = T;
    };

    template<typename R, typename... Args>
    struct decay_function<R(Args...)> {
      using type = R (*)(Args...);
    };

    template<typename R, typename... Args>
    struct decay_function<R(Args..., ...)> {
      using type = R (*)(Args..., ...);
    };
  }  // namespace detail

  template<typename T>
  struct decay {
    using U    = typename remove_reference<T>::type;
    using type = typename detail::decay_array<typename detail::decay_function<typename remove_cv<U>::type>::type>::type;
  };

  template<typename T>
  using decay_t = typename decay<T>::type;
}  // namespace xcore

#endif  //LIB_XCORE_CORE_PORTED_TYPE_TRAITS_HPP
