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
    struct is_nbdelay : xcore::false_type {};

    template<typename C, typename T>
    struct is_nbdelay<C, T, xcore::void_t<decltype(C(xcore::declval<T>(), xcore::declval<T (*)()>()))>>
        : xcore::integral_constant<bool,
                                   xcore::is_same<decltype(static_cast<bool>(xcore::declval<C>())), bool>::value &&
                                     xcore::is_same<decltype(xcore::declval<C>().reset()), void>::value &&
                                     xcore::is_integral<T>::value> {};
  }  // namespace traits

  namespace dummy {
    template<typename TimeType>
    constexpr TimeType get_time() { return 0; }
  }  // namespace dummy

  namespace impl {
    template<template<typename> class SmartDelay, typename TimeType>
    using only_nbdelay = xcore::enable_if_t<traits::is_nbdelay<SmartDelay<TimeType>, TimeType>::value, bool>;

    template<size_t MaxTasks, template<typename> class SmartDelay, typename TimeType, only_nbdelay<SmartDelay, TimeType> = true>
    class task_dispatcher;

    template<template<typename> class SmartDelay, typename TimeType, only_nbdelay<SmartDelay, TimeType> = true>
    class task_t {
    public:
      template<typename T>
      using func_ptr_Targ = void (*)(T *);

      template<typename T>
      using func_ptr_Tref = void (*)(T &);

      using func_ptr      = void      (*)();
      using func_ptr_arg  = func_ptr_Targ<void>;
      using time_func_t   = TimeType();
      using priority_t    = uint8_t;

      struct addition_struct {
        task_t task;
        bool   pred;
      };

      template<size_t MaxTasks, template<typename> class SmartDelay_, typename TimeType_, only_nbdelay<SmartDelay_, TimeType_>>
      friend class task_dispatcher;

    private:
      using smart_delay_t = SmartDelay<TimeType>;

      func_ptr_arg  m_func;
      void         *m_arg;
      smart_delay_t m_sd;
      priority_t    m_priority;

    public:
      task_t()
          : m_func{nullptr}, m_arg(nullptr),
            m_sd{smart_delay_t(0, nullptr)}, m_priority{0} {}

      task_t(const task_t &)     = default;
      task_t(task_t &&) noexcept = default;

      // Function taking argument pointer
      template<typename Arg>
      task_t(func_ptr_Targ<Arg> task_func, Arg *arg, const TimeType interval, time_func_t time_func, const priority_t priority = 0)
          : m_func{functional::ptr_to_void_cast(task_func)}, m_arg(arg),
            m_sd{smart_delay_t(interval, time_func)}, m_priority{priority} {}

      // Function taking argument pointer (no delay)
      template<typename Arg>
      task_t(func_ptr_Targ<Arg> task_func, Arg *arg, const priority_t priority = 0)
          : m_func{functional::ptr_to_void_cast(task_func)}, m_arg(arg),
            m_sd{smart_delay_t(0, dummy::get_time<TimeType>)}, m_priority{priority} {}

      // Function taking nothing
      task_t(const func_ptr task_func, const TimeType interval, time_func_t time_func, const priority_t priority = 0)
          : m_func{reinterpret_cast<func_ptr_arg>(task_func)}, m_arg(nullptr),
            m_sd{smart_delay_t(interval, time_func)}, m_priority{priority} {}

      // Function taking nothing (no delay)
      explicit task_t(const func_ptr task_func, const priority_t priority = 0)
          : m_func{reinterpret_cast<func_ptr_arg>(task_func)}, m_arg(nullptr),
            m_sd{smart_delay_t(0, dummy::get_time<TimeType>)}, m_priority{priority} {}

      task_t &operator=(const task_t &other) {
        if (this == &other) {
          return *this;
        }

        m_func     = other.m_func;
        m_arg      = other.m_arg;
        m_sd       = other.m_sd;
        m_priority = other.m_priority;

        return *this;
      }

      task_t &operator=(task_t &&other) noexcept {
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

    template<size_t MaxTasks, template<typename> class SmartDelay, typename TimeType, only_nbdelay<SmartDelay, TimeType>>
    class task_dispatcher {
      static_assert(MaxTasks > 0, "Scheduler size cannot be zero.");

    private:
      using Task               = task_t<SmartDelay, TimeType>;

      size_t m_size            = {};
      Task   m_tasks[MaxTasks] = {};

    public:
                       task_dispatcher()                            = default;
                       task_dispatcher(const task_dispatcher &)     = default;
                       task_dispatcher(task_dispatcher &&) noexcept = default;

      task_dispatcher &operator+=(typename Task::addition_struct &&task_struct) {
        return this->operator<<(xcore::forward<Task>(task_struct));
      }

      task_dispatcher &operator<<(typename Task::addition_struct &&task_struct) {
        if (task_struct.pred) {
          this->operator<<(xcore::move(task_struct.task));
        }
        return *this;
      }

      task_dispatcher &operator+=(Task &&task) {
        return this->operator<<(xcore::forward<Task>(task));
      }

      task_dispatcher &operator<<(Task &&task) {
        if (m_size < MaxTasks) {
          size_t i;
          for (i = 0; i < m_size && m_tasks[i].m_priority < task.m_priority; ++i)
            ;
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
  }  // namespace impl

  /**
   * Default task type for most frameworks
   */
  using Task = xcore::impl::task_t<xcore::impl::nonblocking_delay, unsigned long>;

  /**
   * Default task dispatcher type for most frameworks
   */
  template<size_t MaxTasks>
  using Dispatcher = xcore::impl::task_dispatcher<MaxTasks, xcore::impl::nonblocking_delay, unsigned long>;
}  // namespace xcore

#endif  //LIB_XCORE_UTILS_TASK_DISPATCHER_HPP
