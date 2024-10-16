#ifndef PORTED_TYPE_TRAITS_HPP
#define PORTED_TYPE_TRAITS_HPP

#include "macros_bootstrap.hpp"

namespace ported {
  typedef decltype(nullptr) nullptr_t;

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

  // IS_SAME

  template<typename, typename>
  struct is_same : false_type {};

  template<typename T>
  struct is_same<T, T> : true_type {};

  template<typename T, typename U>
  inline constexpr bool is_same_v = is_same<T, U>::value;

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

  template<typename>
  struct is_floating_point : false_type {};

  template<>
  struct is_floating_point<float> : true_type {};

  template<>
  struct is_floating_point<double> : true_type {};

  template<>
  struct is_floating_point<long double> : true_type {};

  template<typename>
  struct is_null_pointer : false_type {};

  template<>
  struct is_null_pointer<nullptr_t> : true_type {};

  template<typename T>
  inline constexpr bool is_void_v = is_void<T>::value;

  template<typename T>
  inline constexpr bool is_null_pointer_v = is_null_pointer<T>::value;

  template<typename T>
  inline constexpr bool is_integral_v = is_integral<T>::value;

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
}  // namespace ported

#endif  //PORTED_TYPE_TRAITS_HPP