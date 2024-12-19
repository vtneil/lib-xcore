#ifndef LIB_XCORE_UTILS_TASK_DISPATCHER_HPP
#define LIB_XCORE_UTILS_TASK_DISPATCHER_HPP

#include "utils/nonblocking_delay.hpp"

namespace xcore {
  namespace functional {
    template<typename T>
    constexpr auto ptr_to_void_cast(void (*func)(T *)) -> void (*)(void *) {
      return reinterpret_cast<void (*)(void *)>(func);
    }
  }  // namespace functional

  namespace traits {
    template<typename, typename, typename = void>
    struct is_nbdelay : false_type {};

    template<typename C, typename T>
    struct is_nbdelay<C, T, void_t<decltype(C(xcore::declval<T>(), xcore::declval<T (*)()>()))>>
        : integral_constant<bool,
                            is_same_v<decltype(static_cast<bool>(xcore::declval<C>())), bool> &&
                              is_same_v<decltype(xcore::declval<C>().reset()), void> &&
                              is_integral_v<T>> {};

    template<typename C, typename T>
    inline constexpr bool is_nbdelay_v = is_nbdelay<C, T>::value;
  }  // namespace traits

  namespace dummy {
    template<typename TimeType>
    constexpr TimeType get_time() { return 0; }
  }  // namespace dummy

  namespace detail {
    template<template<typename, bool> class SmartDelay, typename TimeType, bool Adaptive>
    using only_nbdelay = enable_if_t<traits::is_nbdelay_v<SmartDelay<TimeType, Adaptive>, TimeType>, bool>;

    template<size_t MaxTasks, template<typename, bool> class SmartDelay, typename TimeType, bool Adaptive, only_nbdelay<SmartDelay, TimeType, Adaptive> = true>
    class task_dispatcher_impl;

    template<template<typename, bool> class SmartDelay, typename TimeType, bool Adaptive, only_nbdelay<SmartDelay, TimeType, Adaptive> = true>
    struct task_impl {
      template<typename T>
      using func_ptr_Targ = void (*)(T *);

      template<typename T>
      using func_ptr_Tref = void (*)(T &);

      using func_ptr      = void (*)();
      using func_ptr_arg  = func_ptr_Targ<void>;
      using time_func_t   = TimeType();
      using priority_t    = uint8_t;

      struct addition_struct {
        task_impl task;
        bool      pred;
      };

      template<size_t MaxTasks, template<typename, bool> class SmartDelay_, typename TimeType_, bool Adaptive_, only_nbdelay<SmartDelay_, TimeType_, Adaptive_>>
      friend class task_dispatcher_impl;

    private:
      using smart_delay_t = SmartDelay<TimeType, Adaptive>;

      func_ptr_arg  m_func;
      void         *m_arg;
      smart_delay_t m_sd;
      priority_t    m_priority;

    public:
      task_impl()
          : m_func{nullptr}, m_arg(nullptr),
            m_sd{smart_delay_t(0, nullptr)}, m_priority{0} {}

      task_impl(const task_impl &)     = default;
      task_impl(task_impl &&) noexcept = default;

      // Function taking argument pointer
      template<typename Arg>
      task_impl(func_ptr_Targ<Arg> task_func, Arg *arg, const TimeType interval, time_func_t time_func, const priority_t priority = 0)
          : m_func{functional::ptr_to_void_cast(task_func)}, m_arg(arg),
            m_sd{smart_delay_t(interval, time_func)}, m_priority{priority} {}

      // Function taking argument pointer (no delay)
      template<typename Arg>
      task_impl(func_ptr_Targ<Arg> task_func, Arg *arg, const priority_t priority = 0)
          : m_func{functional::ptr_to_void_cast(task_func)}, m_arg(arg),
            m_sd{smart_delay_t(0, dummy::get_time<TimeType>)}, m_priority{priority} {}

      // Function taking nothing
      task_impl(const func_ptr task_func, const TimeType interval, time_func_t time_func, const priority_t priority = 0)
          : m_func{reinterpret_cast<func_ptr_arg>(task_func)}, m_arg(nullptr),
            m_sd{smart_delay_t(interval, time_func)}, m_priority{priority} {}

      // Function taking nothing (no delay)
      explicit task_impl(const func_ptr task_func, const priority_t priority = 0)
          : m_func{reinterpret_cast<func_ptr_arg>(task_func)}, m_arg(nullptr),
            m_sd{smart_delay_t(0, dummy::get_time<TimeType>)}, m_priority{priority} {}

      task_impl &operator=(const task_impl &other) {
        if (this == &other) {
          return *this;
        }

        m_func     = other.m_func;
        m_arg      = other.m_arg;
        m_sd       = other.m_sd;
        m_priority = other.m_priority;

        return *this;
      }

      task_impl &operator=(task_impl &&other) noexcept {
        m_func     = xcore::move(other.m_func);
        m_arg      = xcore::move(other.m_arg);
        m_sd       = xcore::move(other.m_sd);
        m_priority = xcore::move(other.m_priority);

        return *this;
      }

      void operator()() {
        if (m_func != nullptr && m_sd) {
          m_func(m_arg);
        }
      }

      void reset() {
        m_sd.reset();
      }

      addition_struct operator,(const bool pred) const {
        return {xcore::move(*this), pred};
      }

      constexpr TimeType interval() const {
        return m_sd.interval();
      }
    };

    template<size_t MaxTasks, template<typename, bool> class SmartDelay, typename TimeType, bool Adaptive, only_nbdelay<SmartDelay, TimeType, Adaptive>>
    class task_dispatcher_impl {
      static_assert(MaxTasks > 0, "Scheduler size cannot be zero.");

    private:
      using Task               = task_impl<SmartDelay, TimeType, Adaptive>;

      size_t m_size            = {};
      Task   m_tasks[MaxTasks] = {};

    public:
      task_dispatcher_impl()                                 = default;
      task_dispatcher_impl(const task_dispatcher_impl &)     = default;
      task_dispatcher_impl(task_dispatcher_impl &&) noexcept = default;

      task_dispatcher_impl &operator+=(typename Task::addition_struct &&task_struct) {
        return this->operator<<(xcore::forward<Task>(task_struct));
      }

      task_dispatcher_impl &operator<<(typename Task::addition_struct &&task_struct) {
        if (task_struct.pred) {
          this->operator<<(xcore::move(task_struct.task));
        }
        return *this;
      }

      task_dispatcher_impl &operator+=(Task &&task) {
        return this->operator<<(xcore::forward<Task>(task));
      }

      task_dispatcher_impl &operator<<(Task &&task) {
        if (m_size < MaxTasks) {
          size_t i;
          for (i = 0; i < m_size && m_tasks[i].m_priority < task.m_priority; ++i);
          for (size_t j = 0; j < m_size - i; ++j) {
            m_tasks[m_size - j] = xcore::move(m_tasks[m_size - j - 1]);
          }
          m_tasks[i] = xcore::move(task);
          ++m_size;
        }

        return *this;
      }

      void operator()() {
        for (size_t i = 0; i < m_size; ++i) {
          m_tasks[i]();
        }
      }

      void reset() {
        for (size_t i = 0; i < m_size; ++i) {
          m_tasks[i].reset();
        }
      }

      void clear() {
        m_size = 0;
      }

      [[nodiscard]] size_t size() const { return m_size; }

      [[nodiscard]] size_t capacity() const { return MaxTasks; }
    };
  }  // namespace detail

  template<template<typename, bool> class NonblockingT, typename TimeT, bool Adaptive = true>
  using task_t = xcore::detail::task_impl<NonblockingT, TimeT, Adaptive>;

  template<size_t MaxTasks, template<typename, bool> class NonblockingT, typename TimeT, bool Adaptive = true>
  using task_dispatcher = xcore::detail::task_dispatcher_impl<MaxTasks, NonblockingT, TimeT, Adaptive>;

  /**
   * Default task type for most frameworks
   */
  using Task = task_t<xcore::detail::nonblocking_delay_impl, unsigned long, true>;

  /**
   * Default task dispatcher type for most frameworks
   */
  template<size_t MaxTasks>
  using Dispatcher = task_dispatcher<MaxTasks, xcore::detail::nonblocking_delay_impl, unsigned long, true>;
}  // namespace xcore

#endif  //LIB_XCORE_UTILS_TASK_DISPATCHER_HPP
