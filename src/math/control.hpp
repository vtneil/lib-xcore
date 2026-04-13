#ifndef LIB_XCORE_LINALG_CONTROL_HPP
#define LIB_XCORE_LINALG_CONTROL_HPP

#include "core/ported_std.hpp"
#include <algorithm>
#include <limits>

LIB_XCORE_BEGIN_NAMESPACE

template<typename TimeType>
class pid_controller_t {
public:
  using time_func_t = TimeType (*)();

protected:
  real_t kp_{}, ki_{}, kd_{};

  real_t      dt_ = -1;
  TimeType    prev_t_{};
  time_func_t time_func_{nullptr};
  bool        has_prev_time_{false};
  bool        initialized_{false};

  real_t out_min_{::std::numeric_limits<real_t>::lowest()};
  real_t out_max_{::std::numeric_limits<real_t>::max()};

  real_t integrator_{};
  real_t derivative_{};
  real_t prev_e_{};
  real_t prev_meas_{};
  real_t prev_output_{};

  real_t dfilter_tau_{0};  // 0 = disabled

public:
  pid_controller_t(const real_t     &kp,
                   const real_t     &ki,
                   const real_t     &kd,
                   const time_func_t time_func = nullptr,
                   const real_t     &out_min   = ::std::numeric_limits<real_t>::lowest(),
                   const real_t     &out_max   = ::std::numeric_limits<real_t>::max())
      : kp_(kp), ki_(ki), kd_(kd),
        time_func_(time_func),
        out_min_(out_min), out_max_(out_max) {}

  real_t update(const real_t &setpoint, const real_t &measurement, const real_t &dt_new = -1) {
    real_t dt;

    if (time_func_) {
      const auto now = time_func_();
      if (has_prev_time_) {
        dt = static_cast<real_t>(now - prev_t_);
      } else {
        dt             = dt_;
        has_prev_time_ = true;
      }
      prev_t_ = now;
    } else {
      dt = dt_new > 0 ? dt_new : dt_;
    }

    if (dt <= 0) {
      return prev_output_;
    }

    const real_t error = setpoint - measurement;

    if (!initialized_) {
      prev_meas_   = measurement;
      prev_e_      = error;
      derivative_  = 0;
      initialized_ = true;
    }

    const real_t px = kp_ * error;

    integrator_ += error * dt;

    if (ki_ != 0) {
      const real_t i_min = ::std::min(out_min_ / ki_, out_max_ / ki_);
      const real_t i_max = ::std::max(out_min_ / ki_, out_max_ / ki_);
      integrator_        = ::std::clamp(integrator_, i_min, i_max);
    }

    const real_t ix = ki_ * integrator_;

    const real_t raw_d = -(measurement - prev_meas_) / dt;

    if (dfilter_tau_ > 0) {
      const real_t alpha = dfilter_tau_ / (dfilter_tau_ + dt);
      derivative_        = alpha * derivative_ + (1 - alpha) * raw_d;
    } else {
      derivative_ = raw_d;
    }

    const real_t dx = kd_ * derivative_;

    const real_t output = ::std::clamp(px + ix + dx, out_min_, out_max_);

    prev_e_      = error;
    prev_meas_   = measurement;
    prev_output_ = output;
    return output;
  }

  void enable_dfilter(const real_t &tau) {
    dfilter_tau_ = tau > 0 ? tau : 0;
  }

  void disable_dfilter() {
    dfilter_tau_ = 0;
  }

  void update_gains(const real_t &kp, const real_t &ki, const real_t &kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
  }

  void update_dt(const real_t &dt) {
    if (dt > 0) dt_ = dt;
  }

  void update_limits(const real_t &min, const real_t &max) {
    out_min_ = min;
    out_max_ = max;
  }

  void reset() {
    integrator_    = 0;
    derivative_    = 0;
    prev_e_        = 0;
    prev_meas_     = 0;
    prev_output_   = 0;
    initialized_   = false;
    has_prev_time_ = false;
  }

  [[nodiscard]] constexpr real_t derivative() const {
    return derivative_;
  }
};

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_LINALG_CONTROL_HPP
