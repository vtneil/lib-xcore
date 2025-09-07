#ifndef LIB_XCORE_UTILS_SAMPLER_HPP
#define LIB_XCORE_UTILS_SAMPLER_HPP

#include "internal/macros.hpp"
#include "core/ported_std.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<size_t SamplingCount, typename Tp>
class sampler_t {
  static_assert(SamplingCount > 0, "Number of samples must be greater than zero.");

  Tp     fifo_[SamplingCount]{};
  Tp     threshold_{};
  size_t idx_{};
  size_t size_{};
  size_t count_under_{};
  size_t count_over_{};

public:
  sampler_t() = default;

  void add_sample(const Tp &new_entry) {
    if (size_ >= SamplingCount)
      compare_ge(fifo_[idx_], threshold_) ? --count_over_ : --count_under_;
    else
      ++size_;
    fifo_[idx_] = new_entry;
    compare_ge(new_entry, threshold_) ? ++count_over_ : ++count_under_;
    idx_ = (idx_ + 1) % SamplingCount;
  }

  void set_threshold(const Tp &threshold) {
    threshold_   = threshold;
    count_under_ = 0;
    count_over_  = 0;
    for (size_t i = 0; i < size_; ++i)
      compare_ge(fifo_[i], threshold_) ? ++count_over_ : ++count_under_;
  }

  void reset() {
    idx_         = 0;
    size_        = 0;
    count_under_ = 0;
    count_over_  = 0;
  }

  [[nodiscard]] size_t count_under() const {
    return count_under_;
  }

  [[nodiscard]] size_t count_over() const {
    return count_over_;
  }

  template<typename FloatingPoint = double>
  [[nodiscard]] FloatingPoint over_by_under() const {
    if (count_under_ == 0) {
      if (count_over_ == 0) return std::numeric_limits<FloatingPoint>::quiet_NaN();
      return std::numeric_limits<FloatingPoint>::infinity();
    }
    return static_cast<FloatingPoint>(count_over_) / static_cast<FloatingPoint>(count_under_);
  }

  template<typename FloatingPoint = double>
  [[nodiscard]] FloatingPoint under_by_over() const {
    if (count_over_ == 0) {
      if (count_under_ == 0) return std::numeric_limits<FloatingPoint>::quiet_NaN();
      return std::numeric_limits<FloatingPoint>::infinity();
    }
    return static_cast<FloatingPoint>(count_under_) / static_cast<FloatingPoint>(count_over_);
  }

  constexpr static bool compare_ge(const Tp &a, const Tp &b) {
    return a >= b;
  }
};

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_UTILS_SAMPLER_HPP
