#ifndef LIB_XCORE_NETWORK_NAV_HPP
#define LIB_XCORE_NETWORK_NAV_HPP

namespace xcore::network {
  template<auto MicrosTimeFunc>
  class nav_t {
    using TimeT = decltype(MicrosTimeFunc());

  protected:
    TimeT nav_ = {};

  public:
    void update_nav(const TimeT duration) {
      const TimeT curr_time = MicrosTimeFunc();
      const TimeT nav_exp   = curr_time + duration;

      if (nav_exp > nav_)
        nav_ = nav_exp;
    }

    void is_medium_free() {
      const TimeT curr_time = MicrosTimeFunc();

      return curr_time > nav_;
    }
  };
}  // namespace xcore::network

namespace xcore {
  using namespace network;
}

#endif  //LIB_XCORE_NETWORK_NAV_HPP
