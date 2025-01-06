#ifndef LIB_XCORE_CORE_PORTED_INVOKE_HPP
#define LIB_XCORE_CORE_PORTED_INVOKE_HPP

#include "internal/macros.hpp"
#include "core/ported_move.hpp"
#include "core/ported_type_traits.hpp"
#include "core/ported_refwrap.hpp"

LIB_XCORE_BEGIN_NAMESPACE

// INVOKE_RESULT

namespace detail {
  template<typename, typename, typename...>
  struct invoke_result_impl {};

  template<typename F, typename... Args>
  struct invoke_result_impl<F, void_t<decltype(declval<F>()(declval<Args>()...))>, Args...> {
    using type = decltype(declval<F>()(declval<Args>()...));
  };

  template<typename F, typename... Args>
  struct invoke_result : invoke_result_impl<F, void, Args...> {};
}  // namespace detail

template<typename F, typename... Args>
using invoke_result_t = typename detail::invoke_result<F, Args...>::type;

// INVOCABLE

namespace detail {
  template<typename, typename, typename...>
  struct is_invocable_impl : false_type {};

  template<typename F, typename... Args>
  struct is_invocable_impl<F, void_t<decltype(declval<F>()(declval<Args>()...))>, Args...> : true_type {};
}  // namespace detail

template<typename F, typename... Args>
struct is_invocable : detail::is_invocable_impl<F, void, Args...> {};

template<typename F, typename... Args>
inline constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

namespace detail {
  template<typename, typename, typename...>
  struct is_nothrow_invocable_impl : false_type {};

  template<typename F, typename... Args>
  struct is_nothrow_invocable_impl<F, void_t<decltype(static_cast<void>(declval<F>()(declval<Args>()...)))>, Args...>
      : bool_constant<noexcept(declval<F>()(declval<Args>()...))> {};
}  // namespace detail

template<typename F, typename... Args>
struct is_nothrow_invocable : detail::is_nothrow_invocable_impl<F, void, Args...> {};

template<typename F, typename... Args>
inline constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<F, Args...>::value;

// INVOCABLE_R

namespace detail {
  template<typename, typename, typename, typename...>
  struct is_invocable_r_impl : false_type {};

  template<typename R, typename F, typename... Args>
  struct is_invocable_r_impl<R, void_t<decltype(declval<F>()(declval<Args>()...))>, F, Args...>
      : bool_constant<is_convertible_v<decltype(declval<F>()(declval<Args>()...)), R>> {};
}  // namespace detail

template<typename R, typename F, typename... Args>
struct is_invocable_r : detail::is_invocable_r_impl<R, void, F, Args...> {};

template<typename R, typename F, typename... Args>
inline constexpr bool is_invocable_r_v = is_invocable_r<R, F, Args...>::value;

namespace detail {
  template<typename, typename, typename, typename...>
  struct is_nothrow_invocable_r_impl : false_type {};

  template<typename R, typename F, typename... Args>
  struct is_nothrow_invocable_r_impl<R, void_t<decltype(declval<F>()(declval<Args>()...))>, F, Args...>
      : bool_constant<noexcept(declval<F>()(declval<Args>()...)) &&
                      is_convertible_v<decltype(declval<F>()(declval<Args>()...)), R>> {};
}  // namespace detail

template<typename R, typename F, typename... Args>
struct is_nothrow_invocable_r : detail::is_nothrow_invocable_r_impl<R, void, F, Args...> {};

template<typename R, typename F, typename... Args>
inline constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, F, Args...>::value;

// INVOKE

namespace detail {
  template<class>
  constexpr bool is_reference_wrapper_v = false;
  template<class U>
  constexpr bool is_reference_wrapper_v<reference_wrapper<U>> = true;

  template<class T>
  using remove_cvref_t = remove_cv_t<remove_reference_t<T>>;

  template<class C, class Pointed, class Object, class... Args>
  constexpr decltype(auto) invoke_memptr(Pointed C::*member, Object &&object,
                                         Args &&...args) {
    using object_t                    = remove_cvref_t<Object>;
    constexpr bool is_member_function = is_function_v<Pointed>;
    constexpr bool is_wrapped         = is_reference_wrapper_v<object_t>;
    constexpr bool is_derived_object  = is_same_v<C, object_t> || is_base_of_v<C, object_t>;

    if constexpr (is_member_function) {
      if constexpr (is_derived_object)
        return (forward<Object>(object).*member)(forward<Args>(args)...);
      else if constexpr (is_wrapped)
        return (object.get().*member)(forward<Args>(args)...);
      else
        return ((*forward<Object>(object)).*member)(forward<Args>(args)...);
    } else {
      static_assert(is_object_v<Pointed> && sizeof...(args) == 0);
      if constexpr (is_derived_object)
        return forward<Object>(object).*member;
      else if constexpr (is_wrapped)
        return object.get().*member;
      else
        return (*forward<Object>(object)).*member;
    }
  }
}  // namespace detail

template<class F, class... Args>
constexpr invoke_result_t<F, Args...> invoke(F &&f, Args &&...args) noexcept(is_nothrow_invocable_v<F, Args...>) {
  if constexpr (is_member_pointer_v<detail::remove_cvref_t<F>>)
    return detail::invoke_memptr(f, forward<Args>(args)...);
  else
    return forward<F>(f)(forward<Args>(args)...);
}

template<class R, class F, class... Args, enable_if_t<is_invocable_r_v<R, F, Args...>, bool> = true>
constexpr R invoke_r(F &&f, Args &&...args) noexcept(is_nothrow_invocable_r_v<R, F, Args...>) {
  if constexpr (is_void_v<R>)
    invoke(forward<F>(f), forward<Args>(args)...);
  else
    return invoke(forward<F>(f), forward<Args>(args)...);
}
}  // namespace ported

#endif  //LIB_XCORE_CORE_PORTED_INVOKE_HPP
