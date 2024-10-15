#ifndef NONBLOCKING_DELAY_HPP
#define NONBLOCKING_DELAY_HPP

#include "core/ported_std.hpp"

namespace impl {
  template<typename F>
  struct is_procedure : ported::is_invocable_r<void, F> {};

  template<typename F>
  inline constexpr bool is_procedure_v = is_procedure<F>::value;

  template<auto Func>
  struct is_invocable_and_numeric {
    static constexpr bool is_invocable = ported::is_invocable_v<decltype(Func)>;

    static constexpr bool returns_numeric =
      ported::is_integral_v<ported::invoke_result_t<decltype(Func)>> ||
      ported::is_floating_point_v<ported::invoke_result_t<decltype(Func)>>;

    static constexpr bool value = is_invocable && returns_numeric;
  };

  template<auto Func>
  inline constexpr bool is_invocable_and_numeric_v = is_invocable_and_numeric<Func>::value;
}  // namespace impl

template<typename TimeType>
class nonblocking_delay {
public:
  using time_func_t = TimeType();

private:
  time_func_t *m_func            = {};
  TimeType     m_target_interval = {};
  TimeType     m_prev_time       = {};
  TimeType     m_true_interval   = {};

  struct proc_else {
    bool value;

    template<typename Proc, typename = ported::enable_if_t<impl::is_procedure_v<Proc>>>
    void otherwise(Proc &&proc) {
      if (!value) {
        proc();
      }
    }
  };

public:
  nonblocking_delay(const nonblocking_delay &)     = default;

  nonblocking_delay(nonblocking_delay &&) noexcept = default;

  nonblocking_delay(TimeType interval, time_func_t *time_func)
      : m_func{time_func}, m_target_interval{interval}, m_true_interval{interval} {
    if (time_func != nullptr)
      m_prev_time = time_func();
  }

  nonblocking_delay &operator=(const nonblocking_delay &other) {
    if (this == &other) {
      return *this;
    }

    m_func            = other.m_func;
    m_target_interval = other.m_target_interval;
    m_prev_time       = other.m_prev_time;
    m_true_interval   = other.m_true_interval;

    return *this;
  }

  nonblocking_delay &operator=(nonblocking_delay &&other) noexcept {
    m_func            = ported::move(other.m_func);
    m_target_interval = ported::move(other.m_target_interval);
    m_prev_time       = ported::move(other.m_prev_time);
    m_true_interval   = ported::move(other.m_true_interval);

    return *this;
  }

  template<typename Proc, typename = ported::enable_if_t<impl::is_procedure_v<Proc>>>
  proc_else operator()(Proc &&proc) {
    const bool v = this->operator bool();
    if (v) {
      proc();
    }
    return {v};
  }

  bool triggered() {
    return this->operator bool();
  }

  bool passed() {
    return this->operator bool();
  }

  explicit operator bool() {
    if (m_func == nullptr) {
      return false;
    }

    if (m_target_interval == 0) {
      return true;
    }

    // Adaptive interval adjustment
    TimeType curr_time = m_func();
    if (curr_time - m_prev_time >= m_true_interval) {
      // absolute delta_e
      TimeType delta_e = m_target_interval > m_true_interval
                           ? m_target_interval - m_true_interval
                           : m_true_interval - m_target_interval;
      m_true_interval  = delta_e < m_true_interval
                           ? m_true_interval - delta_e
                           : m_true_interval + delta_e;
      m_prev_time      = curr_time;

      return true;
    }

    return false;
  }

  void reset() {
    if (m_func != nullptr) {
      m_prev_time = m_func();
    }
  }

  constexpr TimeType interval() const {
    return m_true_interval;
  }

  void set_interval(const TimeType new_interval) {
    m_target_interval = new_interval;
    m_true_interval   = new_interval;
    this->reset();
  }
};

/**
     * A flip-flop smart delay timer
     *
     * @tparam TimeType
     */
template<typename TimeType, typename = ported::enable_if_t<ported::is_integral_v<TimeType>>>
class on_off_timer {
public:
  using time_func_t = TimeType();
  using NbDelay     = nonblocking_delay<TimeType>;

  struct interval_params {
    TimeType t_on;
    TimeType t_off;
  };

private:
  NbDelay sd_on;
  NbDelay sd_off;
  bool    is_on = false;

public:
  on_off_timer(TimeType interval_on, TimeType interval_off, time_func_t *time_func)
      : sd_on{NbDelay(interval_on, time_func)}, sd_off{NbDelay(interval_off, time_func)} {}

  template<typename Proc, typename = ported::enable_if_t<impl::is_procedure_v<Proc>>>
  on_off_timer &on_rising(Proc &&proc) {
    if (!is_on) {  // if 0
      sd_off([&]() -> void {
        proc();
        is_on = true;
        sd_on.reset();
      });
    }

    return *this;
  }

  template<typename Proc, typename = ported::enable_if_t<impl::is_procedure_v<Proc>>>
  on_off_timer &on_falling(Proc &&proc) {
    if (is_on) {  // if 1
      sd_on([&]() -> void {
        proc();
        is_on = false;
        sd_off.reset();
      });
    }

    return *this;
  }

  TimeType interval_on() {
    return sd_on.interval();
  }

  TimeType interval_off() {
    return sd_off.interval();
  }

  void set_interval_on(const TimeType new_interval) {
    sd_on.set_interval(new_interval);
  }

  void set_interval_off(const TimeType new_interval) {
    sd_off.set_interval(new_interval);
  }

  void reset() {
    sd_on.reset();
    sd_off.reset();
  }
};

#endif  //NONBLOCKING_DELAY_HPP
