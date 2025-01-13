#ifndef LIB_XCORE_UTILS_PIPELINE_HPP
#define LIB_XCORE_UTILS_PIPELINE_HPP

// TODO
// Idea:
// pipeline_t pipe;
// auto result = pipe.run(func1)
//                   .run(func2)
//                   .run(func3);
//
// func1()   -> T1
// func2(T1) -> T2
// func3(T2) -> T3
//
// const T &, T &, T &&

#include "internal/macros.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace detail {
  template<typename T>
  struct pipeline_intermediate {
    explicit constexpr pipeline_intermediate(T &&val) : value(forward<T>(val)) {}

    template<typename Func, typename... Args,
             typename R = invoke_result_t<Func, T, Args...>>
    constexpr auto run(Func func, Args &&...args) && -> pipeline_intermediate<R> {
      return pipeline_intermediate<R>{func(forward<T>(value), xcore::forward<Args>(args)...)};
    }

    constexpr T &&result() && {
      return move(value);
    }

    constexpr operator T() { return value; }

  private:
    T value;
  };

  struct pipeline_start {

    template<typename Func, typename... Args,
             typename R = invoke_result_t<Func, Args...>>
    constexpr auto run(Func func, Args &&...args) && -> pipeline_intermediate<R> {
      return pipeline_intermediate<R>{func(forward<Args>(args)...)};
    }
  };
}  // namespace detail

using pipeline = LIB_XCORE_NAMESPACE::detail::pipeline_start;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_UTILS_PIPELINE_HPP
