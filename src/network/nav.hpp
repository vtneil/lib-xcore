#ifndef LIB_XCORE_NETWORK_NAV_HPP
#define LIB_XCORE_NETWORK_NAV_HPP

namespace xcore::network {
  template<auto MicrosTimeFunc>
  class nav_t {
    using TimeT = decltype(MicrosTimeFunc());

  protected:
    TimeT prev_time_ = {};
    TimeT duration_  = {};

  public:
    void update_nav(const TimeT duration) {
      prev_time_ = MicrosTimeFunc();
      duration_  = duration;
    }

    [[nodiscard]] bool is_medium_free() const {
      const TimeT curr_time = MicrosTimeFunc();
      return curr_time - prev_time_ > duration_;
    }
  };
}  // namespace xcore::network

namespace xcore {
  using namespace network;
}

#endif  //LIB_XCORE_NETWORK_NAV_HPP
