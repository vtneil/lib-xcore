#ifndef LIB_XCORE_NETWORK_NAV_HPP
#define LIB_XCORE_NETWORK_NAV_HPP

#include "internal/macros.hpp"

LIB_XCORE_BEGIN_NAMESPACE

namespace network {
  template<auto MicrosTimeFunc>
  class nav_t {
    using TimeT = decltype(MicrosTimeFunc());

  protected:
    TimeT prev_time_ = {};
    TimeT duration_  = {};

  public:
    void update_nav(const TimeT duration) {
      if (!is_medium_free())
        return;

      prev_time_ = MicrosTimeFunc();
      duration_  = duration;
    }

    [[nodiscard]] bool is_medium_free() const {
      const TimeT curr_time = MicrosTimeFunc();
      return curr_time - prev_time_ > duration_;
    }
  };
}  // namespace network

LIB_XCORE_END_NAMESPACE

LIB_XCORE_BEGIN_NAMESPACE

using namespace network;

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_NETWORK_NAV_HPP
