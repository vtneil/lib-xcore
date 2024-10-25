#ifndef ON_OFF_TIMER_HPP
#define ON_OFF_TIMER_HPP

#include "utils/nonblocking_delay.hpp"

namespace impl {
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

    template<typename Proc, typename = ported::enable_if_t<detail::is_procedure_v<Proc>>>
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

    template<typename Proc, typename = ported::enable_if_t<detail::is_procedure_v<Proc>>>
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
}  // namespace impl

template<typename TimeT>
using on_off_timer = impl::on_off_timer<TimeT>;

/**
 * Default flip-flip timer type for most frameworks
 */
using FfTimer = on_off_timer<unsigned long>;

#endif  //ON_OFF_TIMER_HPP
