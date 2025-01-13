#ifndef LIB_XCORE_UTILS_ON_OFF_TIMER_HPP
#define LIB_XCORE_UTILS_ON_OFF_TIMER_HPP

#include "internal/macros.hpp"
#include "utils/nonblocking_delay.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace detail {
  /**
     * A flip-flop smart delay timer
     *
     * @tparam TimeType
     */
  template<typename TimeType, bool Adaptive, typename = enable_if_t<is_integral_v<TimeType>>>
  class on_off_timer_impl {
  public:
    using time_func_t = TimeType();
    using NbDelay     = nonblocking_delay_impl<TimeType, Adaptive>;

    struct interval_params {
      TimeType t_on;
      TimeType t_off;
    };

  private:
    NbDelay sd_on;
    NbDelay sd_off;
    bool    is_on = false;

  public:
    on_off_timer_impl(TimeType interval_on, TimeType interval_off, time_func_t *time_func)
        : sd_on{NbDelay(interval_on, time_func)}, sd_off{NbDelay(interval_off, time_func)} {}

    template<typename Proc, typename = enable_if_t<is_procedure_v<Proc>>>
    on_off_timer_impl &on_rising(Proc &&proc) {
      if (!is_on) {  // if 0
        sd_off([&]() -> void {
          proc();
          is_on = true;
          sd_on.reset();
        });
      }

      return *this;
    }

    template<typename Proc, typename = enable_if_t<is_procedure_v<Proc>>>
    on_off_timer_impl &on_falling(Proc &&proc) {
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
}  // namespace detail

template<typename TimeT, bool Adaptive = true>
using on_off_timer = LIB_XCORE_NAMESPACE::detail::on_off_timer_impl<TimeT, Adaptive>;

/**
   * Default flip-flip timer type for most frameworks
   */
using FfTimer = on_off_timer<unsigned long>;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_UTILS_ON_OFF_TIMER_HPP
