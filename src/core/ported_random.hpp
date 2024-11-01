#ifndef LIB_XCORE_CORE_PORTED_RANDOM_HPP
#define LIB_XCORE_CORE_PORTED_RANDOM_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cmath>

#include "core/ported_type_traits.hpp"

namespace xcore {
  // Pseudo-random number engines

  template<typename UIntType, UIntType a, UIntType c, UIntType m>
  class linear_congruential_engine {
    static_assert(is_unsigned_v<UIntType>);

  public:
    using result_type = UIntType;

    // Member constexpr constants

    static constexpr UIntType multiplier   = a;
    static constexpr UIntType increment    = c;
    static constexpr UIntType modulus      = m;
    static constexpr UIntType default_seed = 1u;

    // Constructors

    explicit linear_congruential_engine(const result_type seed = default_seed) : state_(seed) {}

    // Member functions

    static constexpr result_type min() {
      return 0;
    }

    static constexpr result_type max() {
      return m - 1;
    }

    void seed(result_type seed) {
      state_ = seed;
    }

    result_type operator()() {
      state_ = (a * state_ + c) % m;
      return state_;
    }

    void discard(const size_t z) {
      for (size_t i = 0; i < z; ++i) {
        (*this)();
      }
    }

  private:
    result_type state_;
  };

  template<typename UIntType, size_t w, UIntType n, UIntType m, UIntType r,
           UIntType a, UIntType u, UIntType d, UIntType s,
           UIntType b, UIntType t,
           UIntType c, UIntType l, UIntType f>
  class mersenne_twister_engine {
    static_assert(is_unsigned_v<UIntType>);

  public:
    using result_type = UIntType;

    // Member constexpr constants

    static constexpr UIntType word_size                 = w;
    static constexpr UIntType state_size                = n;
    static constexpr UIntType shift_size                = m;
    static constexpr UIntType mask_bits                 = r;
    static constexpr UIntType xor_mask                  = a;
    static constexpr UIntType tampering_u               = u;
    static constexpr UIntType tampering_d               = d;
    static constexpr UIntType tampering_s               = s;
    static constexpr UIntType tampering_b               = b;
    static constexpr UIntType tampering_t               = t;
    static constexpr UIntType tampering_c               = c;
    static constexpr UIntType tampering_l               = l;
    static constexpr UIntType initialization_multiplier = f;
    static constexpr UIntType default_seed              = 5489u;

    // Constructors

    explicit mersenne_twister_engine(const result_type seed = default_seed) {
      seed_engine(seed);
    }

    // Member functions

    static constexpr result_type min() {
      return 0;
    }

    static constexpr result_type max() {
      return ~result_type(0);
    }

    void seed(const result_type seed) {
      seed_engine(seed);
    }

    result_type operator()() {
      if (index_ >= n) {
        twist();
      }

      result_type y = MT_[index_++];
      y ^= (y >> u) & d;
      y ^= (y << s) & b;
      y ^= (y << t) & c;
      y ^= y >> l;

      return y;
    }

    void discard(const size_t z) {
      for (size_t i = 0; i < z; ++i) {
        (*this)();
      }
    }

  private:
    result_type MT_[n];
    size_t      index_ = n;

  private:
    void seed_engine(const result_type seed) {
      MT_[0] = seed;
      for (size_t i = 1; i < n; ++i) {
        MT_[i] = (f * (MT_[i - 1] ^ (MT_[i - 1] >> (w - 2))) + i) & d;
      }
    }

    void twist() {
      for (size_t i = 0; i < n; ++i) {
        result_type x  = (MT_[i] & upper_mask()) + (MT_[(i + 1) % n] & lower_mask());
        result_type xA = x >> 1;
        if (x % 2 != 0) {
          xA ^= a;
        }
        MT_[i] = MT_[(i + m) % n] ^ xA;
      }
      index_ = 0;
    }

    static constexpr result_type upper_mask() {
      return (~result_type(0)) << r;
    }
    static constexpr result_type lower_mask() {
      return ~upper_mask();
    }
  };

  template<class UIntType, size_t w, size_t s, size_t r>
  class subtract_with_carry_engine {
    static_assert(is_unsigned_v<UIntType>);

  public:
    using result_type = UIntType;

    // Member constexpr constants

    static constexpr UIntType word_size    = w;
    static constexpr UIntType short_lag    = s;
    static constexpr UIntType long_lag     = r;
    static constexpr UIntType default_seed = 19780503u;

    // Constructors

    explicit subtract_with_carry_engine(const result_type seed = default_seed) {
      seed_engine(seed);
    }

    // Member functions

    static constexpr result_type min() {
      return 0;
    }
    static constexpr result_type max() {
      return (UIntType(1) << w) - 1;
    }

    void seed(const result_type seed) {
      seed_engine(seed);
    }

    result_type operator()() {
      long ps = pos_ - s;
      if (ps < 0)
        ps += r;

      result_type xi;
      if (state_[ps] >= state_[pos_] + carry_) {
        xi     = state_[ps] - state_[pos_] - carry_;
        carry_ = 0;
      } else {
        xi     = (UIntType(1) << w) - state_[pos_] - carry_ + state_[ps];
        carry_ = 1;
      }
      state_[pos_] = xi;

      if (++pos_ >= r) {
        pos_ = 0;
      }

      return xi;
    }

    void discard(const size_t z) {
      for (size_t i = 0; i < z; ++i) {
        (*this)();
      }
    }

  private:
    result_type state_[r];
    result_type carry_ = 0;
    size_t      pos_   = 0;

  private:
    void seed_engine(const result_type seed) {
      // LCG
      linear_congruential_engine<uint_least32_t, 40014u, 0u, 2147483563u> lcg(seed == 0 ? default_seed : seed % 2147483563u);

      // N
      const size_t n = (w + 31) / 32;

      for (size_t i = 0; i < r; ++i) {
        UIntType sum    = 0u;
        UIntType factor = 1u;
        for (size_t j = 0; j < n; ++j) {
          sum = (sum + (static_cast<UIntType>(lcg()) * factor)) & max();
          factor <<= 32;
        }
        state_[i] = sum & max();
      }
      carry_ = state_[r - 1] == 0 ? 1 : 0;
      pos_   = 0;
    }
  };

  // Engine Adaptors

  template<class Engine, size_t p, size_t r>
  class discard_block_engine {
  public:
    using result_type = typename Engine::result_type;

    // Constructors

    explicit discard_block_engine(result_type seed = 1) : engine_(seed), used_(0) {}

    // Member functions

    static constexpr result_type min() {
      return Engine::min();
    }
    static constexpr result_type max() {
      return Engine::max();
    }

    void seed(result_type seed) {
      engine_.seed(seed);
      used_ = 0;
    }

    result_type operator()() {
      if (used_ == r) {
        _discard();
        used_ = 0;
      }
      ++used_;
      return engine_();
    }

    void discard(const size_t z) {
      for (size_t i = 0; i < z; ++i) {
        (*this)();
      }
    }

  private:
    Engine engine_;
    size_t used_;

  private:
    void _discard() {
      for (size_t i = 0; i < p - r; ++i) {
        engine_();
      }
    }
  };

  template<class Engine, size_t w, class UIntType = typename Engine::result_type>
  class independent_bits_engine {
  public:
    using result_type = UIntType;

    // Constructors

    explicit independent_bits_engine(result_type seed = 1) : engine_(seed) {}

    // Member functions

    static constexpr result_type min() {
      return 0;
    }
    static constexpr result_type max() {
      return (UIntType(1) << w) - 1;
    }

    void seed(result_type seed) {
      engine_.seed(seed);
    }

    result_type operator()() {
      result_type result         = 0;
      unsigned    bits_generated = 0;

      // Generate w bits by accumulating values from the base engine
      while (bits_generated < w) {
        UIntType       value        = engine_() - engine_.min();
        const unsigned bits_to_take = w - bits_generated < engine_bits() ? w - bits_generated : engine_bits();
        result                      = (result << bits_to_take) | (value & ((UIntType(1) << bits_to_take) - 1));
        bits_generated += bits_to_take;
      }

      return result & max();
    }

    void discard(const size_t z) {
      for (size_t i = 0; i < z; ++i) {
        (*this)();
      }
    }

  private:
    Engine engine_;

  private:
    static constexpr unsigned engine_bits() {
      UIntType range = Engine::max() - Engine::min();
      unsigned bits  = 0;
      while (range > 0) {
        range >>= 1;
        ++bits;
      }
      return bits;
    }
  };

  template<class Engine, size_t k>
  class shuffle_order_engine {
  public:
    using result_type = typename Engine::result_type;

    // Constructors

    explicit shuffle_order_engine(result_type seed = 1) : engine_(seed) {
      seed_engine();
    }

    // Member functions

    static constexpr result_type min() {
      return Engine::min();
    }

    static constexpr result_type max() {
      return Engine::max();
    }

    void seed(result_type seed) {
      engine_.seed(seed);
      seed_engine();
    }

    result_type operator()() {
      constexpr result_type range = max() - min();
      size_t                j     = k;
      const result_type     y     = index_ - min();
      j *= y / (range + 1.0);
      index_    = table_[j];
      table_[j] = engine_();
      return index_;
    }

    void discard(const size_t z) {
      for (size_t i = 0; i < z; ++i) {
        (*this)();
      }
    }

  private:
    Engine      engine_;
    result_type table_[k];
    size_t      index_;

  private:
    void seed_engine() {
      for (size_t i = 0; i < k; ++i)
        table_[i] = engine_();
      index_ = engine_();
    }
  };

  // Standard pseudo-random number generator engine

  using minstd_rand   = linear_congruential_engine<uint32_t, 48271ul, 0ul, 2147483647ul>;
  using minstd_rand0  = linear_congruential_engine<uint32_t, 16807ul, 0ul, 2147483647ul>;
  using mt19937       = mersenne_twister_engine<uint32_t, 32ul, 624ul, 397ul, 31ul, 0x9908b0dful, 11ul, 0xfffffffful, 7, 0x9d2c5680ul, 15ul, 0xefc60000ul, 18ul, 1812433253ul>;
  using mt19937_64    = mersenne_twister_engine<uint64_t, 64ull, 312ull, 156ull, 31ull, 0xb5026f5aa96619e9ull, 29ull, 0x5555555555555555ull, 17ull, 0x71d67fffeda60000ull, 37ull, 0xfff7eee000000000ull, 43ull, 6364136223846793005ull>;
  using ranlux24_base = subtract_with_carry_engine<uint32_t, 24ul, 10ul, 24ul>;
  using ranlux48_base = subtract_with_carry_engine<uint64_t, 48ul, 5ul, 12ul>;
  using ranlux24      = discard_block_engine<ranlux24_base, 223ul, 23ul>;
  using ranlux48      = discard_block_engine<ranlux48_base, 389ull, 11ull>;
  using knuth_b       = shuffle_order_engine<minstd_rand0, 256ul>;

  typedef minstd_rand0 default_random_engine;

  // Distributions (Common)

  template<typename IntType = int>
  class uniform_int_distribution {
    static_assert(is_integral_v<IntType>);

  public:
    using result_type = IntType;

    // Constructors

    explicit uniform_int_distribution(result_type a = 0, result_type b = max_integral<result_type>())
        : a_(a), b_(b) {}

    // Member functions

    template<typename Engine>
    result_type operator()(Engine &engine) {
      return a_ + (engine() % (b_ - a_ + 1));
    }

    void reset() {
    }

    result_type min() const {
      return a_;
    }

    result_type max() const {
      return b_;
    }

  private:
    result_type a_;
    result_type b_;
  };

  template<typename RealType = double>
  class uniform_real_distribution {
    static_assert(is_floating_point_v<RealType>);

  public:
    using result_type = RealType;

    explicit uniform_real_distribution(result_type a = 0.0, result_type b = 1.0) : a_(a), b_(b) {}

    template<typename Engine>
    result_type operator()(Engine &engine) {
      const auto        m = static_cast<result_type>(engine() - engine.min());
      const auto        n = static_cast<result_type>(engine.max() - engine.min());
      const result_type r = m / n;
      const result_type d = b_ - a_;
      return a_ + r * d;
    }

    void reset() {
    }

    result_type min() const {
      return a_;
    }

    result_type max() const {
      return b_;
    }

  private:
    result_type a_;
    result_type b_;
  };

  class bernoulli_distribution {
  public:
    using result_type = bool;

    explicit bernoulli_distribution(const double p = 0.5) : probability_(p) {}

    template<typename Engine>
    result_type operator()(Engine &engine) {
      const auto   m = static_cast<double>(engine() - engine.min());
      const auto   n = static_cast<double>(engine.max() - engine.min());
      const double r = m / n;
      return r < probability_;
    }

    void reset() {
    }

  private:
    double probability_;
  };

  template<typename IntType = int>
  class binomial_distribution {
    static_assert(is_integral_v<IntType>);

  public:
    using result_type = IntType;

    explicit binomial_distribution(IntType t = 1, const double p = 0.5) : trials_(t), probability_(p) {}

    template<typename Engine>
    result_type operator()(Engine &engine) {
      result_type            successes = 0;
      bernoulli_distribution bernoulli_trial(probability_);
      for (IntType i = 0; i < trials_; ++i) {
        if (bernoulli_trial(engine)) {
          ++successes;
        }
      }
      return successes;
    }

    void reset() {
    }

  private:
    IntType trials_;
    double  probability_;
  };

  template<typename RealType = double>
  class normal_distribution {
    static_assert(is_floating_point_v<RealType>);

  public:
    using result_type = RealType;

    explicit normal_distribution(result_type mean = 0.0, result_type stddev = 1.0)
        : mean_(mean), stddev_(stddev), has_spare_(false) {}

    template<typename Engine>
    result_type operator()(Engine &engine) {
      if (has_spare_) {
        has_spare_ = false;
        return spare_ * stddev_ + mean_;
      } else {
        double u, v, s;
        do {
          u = static_cast<RealType>(engine() - engine.min()) / static_cast<RealType>(engine.max() - engine.min()) * 2.0 - 1.0;
          v = static_cast<RealType>(engine() - engine.min()) / static_cast<RealType>(engine.max() - engine.min()) * 2.0 - 1.0;
          s = u * u + v * v;
        } while (s >= 1 || s == 0);

        s          = sqrt(-2.0 * log(s) / s);
        spare_     = v * s;
        has_spare_ = true;
        return u * s * stddev_ + mean_;
      }
    }

    void        reset() { has_spare_ = false; }

    result_type mean() const { return mean_; }
    result_type stddev() const { return stddev_; }

  private:
    result_type mean_;
    result_type stddev_;
    result_type spare_;
    bool        has_spare_;
  };
}  // namespace xcore

#endif  //LIB_XCORE_CORE_PORTED_RANDOM_HPP
