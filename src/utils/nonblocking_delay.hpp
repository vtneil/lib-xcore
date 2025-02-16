#ifndef LIB_XCORE_UTILS_NONBLOCKING_DELAY_HPP
#define LIB_XCORE_UTILS_NONBLOCKING_DELAY_HPP

#include "internal/macros.hpp"
#include "core/ported_std.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace detail {
  template<typename F>
  struct is_procedure : is_invocable_r<void, F> {};

  template<typename F>
  inline constexpr bool is_procedure_v = is_procedure<F>::value;

  template<auto Func>
  struct is_invocable_and_numeric {
    static constexpr bool is_invocable = is_invocable_v<decltype(Func)>;

    static constexpr bool returns_numeric =
      is_integral_v<invoke_result_t<decltype(Func)>> ||
      is_floating_point_v<invoke_result_t<decltype(Func)>>;

    static constexpr bool value = is_invocable && returns_numeric;
  };

  template<auto Func>
  inline constexpr bool is_invocable_and_numeric_v = is_invocable_and_numeric<Func>::value;
}  // namespace detail

namespace detail {
  template<typename TimeType, bool Adaptive, bool AutoReset = true>
  class nonblocking_delay_impl {
  public:
    using time_func_t = TimeType();

  private:
    time_func_t *func_            = {};
    TimeType     target_interval_ = {};
    TimeType     prev_time_       = {};
    TimeType     gate_interval_   = {};

    struct proc_else {
      nonblocking_delay_impl &parent;
      bool                    value;

      template<typename Proc, typename = enable_if_t<is_procedure_v<Proc>>>
      void otherwise(Proc &&proc) {
        if (!value) {
          proc();
        }
      }
    };

  public:
    nonblocking_delay_impl(const nonblocking_delay_impl &)     = default;

    nonblocking_delay_impl(nonblocking_delay_impl &&) noexcept = default;

    nonblocking_delay_impl(TimeType interval, time_func_t *time_func)
        : func_{time_func}, target_interval_{interval}, gate_interval_{interval} {
      if (time_func != nullptr)
        prev_time_ = time_func();
    }

    nonblocking_delay_impl &operator=(const nonblocking_delay_impl &other) {
      if (this == &other) {
        return *this;
      }

      func_            = other.func_;
      target_interval_ = other.target_interval_;
      prev_time_       = other.prev_time_;
      gate_interval_   = other.gate_interval_;

      return *this;
    }

    nonblocking_delay_impl &operator=(nonblocking_delay_impl &&other) noexcept {
      func_            = move(other.func_);
      target_interval_ = move(other.target_interval_);
      prev_time_       = move(other.prev_time_);
      gate_interval_   = move(other.gate_interval_);

      return *this;
    }

    template<typename Proc, typename = enable_if_t<is_procedure_v<Proc>>>
    proc_else operator()(Proc &&proc) {
      const bool v = _op_bool_impl();
      if (v) {
        proc();
      }
      return {*this, v};
    }

    bool triggered() {
      return _op_bool_impl();
    }

    bool passed() {
      return _op_bool_impl();
    }

    explicit operator bool() {
      return _op_bool_impl();
    }

    void reset() {
      if (func_ != nullptr) {
        prev_time_ = func_();
      }
    }

    constexpr TimeType interval() const {
      return gate_interval_;
    }

    void set_interval(const TimeType new_interval) {
      target_interval_ = new_interval;
      gate_interval_   = new_interval;
      this->reset();
    }

  protected:
    bool _op_bool_impl() {
      if (this->func_ == nullptr) {
        return false;
      }

      if (this->target_interval_ == 0) {
        return true;
      }

      TimeType curr_time = this->func_();

      if (curr_time - this->prev_time_ >= this->gate_interval_) {
        if constexpr (Adaptive) {
          // Adaptive interval adjustment
          // Adjust gate interval to match target interval
          TimeType delta_e     = this->target_interval_ > this->gate_interval_
                                   ? this->target_interval_ - this->gate_interval_
                                   : this->gate_interval_ - this->target_interval_;
          this->gate_interval_ = delta_e < this->gate_interval_
                                   ? this->gate_interval_ - delta_e
                                   : this->gate_interval_ + delta_e;
        }
        this->prev_time_ = curr_time;
        return true;
      }

      return false;
    }
  };

  template<typename TimeType, bool Adaptive>
  class nonblocking_delay_impl<TimeType, Adaptive, false> : nonblocking_delay_impl<TimeType, Adaptive> {
    using BaseType = nonblocking_delay_impl<TimeType, Adaptive>;

  public:
    // Inherit constructor
    using BaseType::BaseType;

  protected:
    // Override by hiding
    bool _op_bool_impl() {
      if (this->func_ == nullptr) {
        return false;
      }

      if (this->target_interval_ == 0) {
        return true;
      }

      TimeType curr_time = this->func_();

      if (curr_time - this->prev_time_ >= this->gate_interval_) {
        if constexpr (Adaptive) {
          // Adaptive interval adjustment
          // Adjust gate interval to match target interval
          TimeType delta_e     = this->target_interval_ > this->gate_interval_
                                   ? this->target_interval_ - this->gate_interval_
                                   : this->gate_interval_ - this->target_interval_;
          this->gate_interval_ = delta_e < this->gate_interval_
                                   ? this->gate_interval_ - delta_e
                                   : this->gate_interval_ + delta_e;
        }
        return true;
      }

      return false;
    }
  };
}  // namespace detail

template<typename TimeT, bool Adaptive = true>
using nonblocking_delay = LIB_XCORE_NAMESPACE::detail::nonblocking_delay_impl<TimeT, Adaptive>;

template<typename TimeT>
using timeout_timer = LIB_XCORE_NAMESPACE::detail::nonblocking_delay_impl<TimeT, false, false>;

/**
 * Default non-blocking delay type for most frameworks
 */
using NbDelay = nonblocking_delay<unsigned long>;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_UTILS_NONBLOCKING_DELAY_HPP
