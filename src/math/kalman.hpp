#ifndef LIB_XCORE_LINALG_KALMAN_HPP
#define LIB_XCORE_LINALG_KALMAN_HPP

#include "core/ported_std.hpp"
#include "./numeric_vector.hpp"
#include "./numeric_matrix.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
class kalman_filter_t {
private:
  // Note that numeric_matrix<N_, M_> maps from R_^M_ to R_^N_
  static constexpr size_t N_ = StateVectorDimension;        // ALias
  static constexpr size_t M_ = MeasurementVectorDimension;  // Alias
  static constexpr size_t L_ = ControlVectorDimension;      // Alias

protected:
  numeric_matrix<N_, N_>       &F_;  // state-transition model
  const numeric_matrix<N_, L_> &B_;  // control-input model
  const numeric_matrix<M_, N_> &H_;  // measurement model
  numeric_matrix<N_, N_>       &Q_;  // covariance of the process noise
  numeric_matrix<M_, M_>       &R_;  // covariance of the measurement noise
  numeric_vector<N_>            x_;  // state vector
  numeric_matrix<N_, N_>        P_;  // state covariance, self-initialized as Q_

public:
  /**
   * Simple Kalman filter
   *
   * @param F_matrix state-transition model
   * @param B_matrix control-input model
   * @param H_matrix measurement model
   * @param Q_matrix covariance of the process noise
   * @param R_matrix covariance of the measurement noise
   * @param x0 initial state vector
   * @param P0 initial P matrix
   */
  constexpr kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    numeric_matrix<N_, N_>       &Q_matrix,
    numeric_matrix<M_, M_>       &R_matrix,
    const numeric_vector<N_>     &x0,
    const numeric_matrix<N_, N_> &P0)
      : F_{F_matrix}, B_{B_matrix}, H_{H_matrix},
        Q_{Q_matrix}, R_{R_matrix}, x_{x0}, P_{P0} {}

  /**
   * Simple Kalman filter
   *
   * @param F_matrix state-transition model
   * @param B_matrix control-input model
   * @param H_matrix measurement model
   * @param Q_matrix covariance of the process noise
   * @param R_matrix covariance of the measurement noise
   * @param x_0 initial state vector
   */
  constexpr kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    const numeric_matrix<N_, N_> &Q_matrix,
    const numeric_matrix<M_, M_> &R_matrix,
    const numeric_vector<N_>     &x_0)
      : kalman_filter_t(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x_0, Q_matrix) {}

  constexpr kalman_filter_t(const kalman_filter_t &) = default;

  constexpr kalman_filter_t(kalman_filter_t &&) noexcept = default;

  kalman_filter_t &operator=(const kalman_filter_t &) = default;

  kalman_filter_t &operator=(kalman_filter_t &&) noexcept = default;

  virtual ~kalman_filter_t() = default;

  virtual kalman_filter_t &predict() {
    return this->predict({});
  }

  /**
   * Kalman filter prediction
   *
   * @param u control input vector
   */
  virtual kalman_filter_t &predict(const numeric_vector<L_> &u) {
    x_ = F_ * x_ + B_ * u;
    P_ = F_ * P_.matmul_T(F_) + Q_;

    // Symmetrize -> Makes P SPD
    P_ = 0.5 * (P_ + P_.transpose());
    return *this;
  }

  /**
   * Kalman filter update
   *
   * @param z Measurement vector
   */
  virtual kalman_filter_t &update(const numeric_vector<M_> &z) {
    const numeric_vector<M_>     y   = z - this->H_ * this->x_;
    const numeric_matrix<N_, M_> PHT = this->P_.matmul_T(this->H_);
    const numeric_matrix<M_, M_> S   = this->H_ * PHT + this->R_;
    const numeric_matrix<N_, M_> K   = PHT * S.inverse();

    // Update state
    this->x_ += K * y;

    // Joseph form
    const numeric_matrix<N_, N_> I_KH = numeric_matrix<N_, N_>::identity() - K * this->H_;
    this->P_                          = I_KH * this->P_ * I_KH.transpose() + K * this->R_ * K.transpose();

    // Symmetrize -> Makes P SPD
    this->P_ = 0.5 * (this->P_ + this->P_.transpose());

    return *this;
  }

  virtual kalman_filter_t &operator<<(const numeric_vector<M_> &z) {
    return this->predict().update(z);
  }

  template<typename... Ts>
  kalman_filter_t &update(Ts... vs) { return update(make_numeric_vector<M_>({vs...})); }

  const numeric_vector<N_>     &state_vector = x_;
  const real_t                 &state        = x_[0];
  const numeric_matrix<M_, M_> &R            = R_;
  const numeric_matrix<N_, N_> &Q            = Q_;
};

template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
class iae_kalman_filter_t final : public kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension> {
private:
  // Note that numeric_matrix<N_, M_> maps from R_^M_ to R_^N_
  static constexpr size_t N_ = StateVectorDimension;        // ALias
  static constexpr size_t M_ = MeasurementVectorDimension;  // Alias
  static constexpr size_t L_ = ControlVectorDimension;      // Alias

  using Base = kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension>;

protected:
  const real_t alpha_;  // EMA Smoothing factor for R
  const real_t beta_;   // EMA Smoothing factor for Q

public:
  /**
   * Simple Kalman filter
   *
   * @param F_matrix state-transition model
   * @param B_matrix control-input model
   * @param H_matrix measurement model
   * @param Q_matrix covariance of the process noise
   * @param R_matrix covariance of the measurement noise
   * @param x0 initial state vector
   * @param P0 initial P matrix
   * @param alpha EMA Smoothing factor for R
   * @param beta EMA Smoothing factor for Q
   */
  constexpr iae_kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    numeric_matrix<N_, N_>       &Q_matrix,
    numeric_matrix<M_, M_>       &R_matrix,
    const numeric_vector<N_>     &x0,
    const numeric_matrix<N_, N_> &P0,
    const real_t                 &alpha = 0.1,
    const real_t                 &beta  = 0.1)
      : Base(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x0, P0),
        alpha_{alpha}, beta_{beta} {}

  /**
   * Simple Kalman filter
   *
   * @param F_matrix state-transition model
   * @param B_matrix control-input model
   * @param H_matrix measurement model
   * @param Q_matrix covariance of the process noise
   * @param R_matrix covariance of the measurement noise
   * @param x_0 initial state vector
   * @param alpha EMA Smoothing factor for R
   * @param beta EMA Smoothing factor for Q
   */
  constexpr iae_kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    numeric_matrix<N_, N_>       &Q_matrix,
    numeric_matrix<M_, M_>       &R_matrix,
    const numeric_vector<N_>     &x_0,
    const real_t                 &alpha = 0.1,
    const real_t                 &beta  = 0.1)
      : iae_kalman_filter_t(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x_0, Q_matrix, alpha, beta) {}

  constexpr iae_kalman_filter_t(const iae_kalman_filter_t &) = default;

  constexpr iae_kalman_filter_t(iae_kalman_filter_t &&) noexcept = default;

  iae_kalman_filter_t &operator=(const iae_kalman_filter_t &) = default;

  iae_kalman_filter_t &operator=(iae_kalman_filter_t &&) noexcept = default;

  /**
   * Kalman filter update
   *
   * @param z Measurement vector
   */
  iae_kalman_filter_t &update(const numeric_vector<M_> &z) override {
    const numeric_vector<M_>     y   = z - this->H_ * this->x_;
    const numeric_matrix<N_, M_> PHT = this->P_.matmul_T(this->H_);
    const numeric_matrix<M_, M_> S   = this->H_ * PHT + this->R_;
    const numeric_matrix<N_, M_> K   = PHT * S.inverse();

    // Update state
    this->x_ += K * y;

    // Joseph form
    const numeric_matrix<N_, N_> I_KH = numeric_matrix<N_, N_>::identity() - K * this->H_;

    this->P_ = I_KH * this->P_ * I_KH.transpose() + K * this->R_ * K.transpose();

    // Symmetrize -> Makes P SPD
    this->P_ = 0.5 * (this->P_ + this->P_.transpose());

    const numeric_matrix<M_, 1>  y_mat = y.as_matrix_col();
    const numeric_matrix<M_, M_> y_yT  = y_mat.matmul_T(y_mat);

    // Adapt R
    const numeric_matrix<M_, M_> HPHT = this->H_ * PHT;
    this->R_                          = (1. - alpha_) * this->R_ + alpha_ * (y_yT - HPHT);

    // Adapt Q
    this->Q_ = (1 - beta_) * this->Q_ + beta_ * (K * y_yT * K.transpose());

    return *this;
  }
};

template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
class r_iae_kalman_filter_t final
    : public kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension> {
private:
  static constexpr size_t N_ = StateVectorDimension;
  static constexpr size_t M_ = MeasurementVectorDimension;
  static constexpr size_t L_ = ControlVectorDimension;

  using Base = kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension>;

protected:
  const real_t alpha_;  // EMA smoothing for R
  const real_t beta_;   // EMA smoothing for Q
  const real_t tau_;    // Huber threshold (≈ “k-sigma” in Mahalanobis space)
  const real_t eps_;    // small jitter to avoid div-by-zero

public:
  /**
   * Robust IAE Kalman filter (R-IAE)
   *
   * @param F_matrix state-transition model
   * @param B_matrix control-input model
   * @param H_matrix measurement model
   * @param Q_matrix process noise covariance (modifiable)
   * @param R_matrix measurement noise covariance (modifiable)
   * @param x0       initial state
   * @param P0       initial covariance
   * @param alpha    EMA smoothing factor for R (0..1)
   * @param beta     EMA smoothing factor for Q (0..1)
   * @param tau      Huber threshold in Mahalanobis units (e.g., 3.0)
   * @param eps      small positive number to stabilize divisions (e.g., 1e-12)
   */
  constexpr r_iae_kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    numeric_matrix<N_, N_>       &Q_matrix,
    numeric_matrix<M_, M_>       &R_matrix,
    const numeric_vector<N_>     &x0,
    const numeric_matrix<N_, N_> &P0,
    const real_t                 &alpha = 0.1,
    const real_t                 &beta  = 0.1,
    const real_t                 &tau   = 3.0,
    const real_t                 &eps   = 1.e-12)
      : Base(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x0, P0),
        alpha_{alpha}, beta_{beta}, tau_{tau}, eps_{eps} {}

  /**
   * Convenience ctor (P0 defaults to Q)
   */
  constexpr r_iae_kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    numeric_matrix<N_, N_>       &Q_matrix,
    numeric_matrix<M_, M_>       &R_matrix,
    const numeric_vector<N_>     &x0,
    const real_t                 &alpha = 0.1,
    const real_t                 &beta  = 0.1,
    const real_t                 &tau   = 3.0,
    const real_t                 &eps   = 1.e-12)
      : r_iae_kalman_filter_t(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x0, Q_matrix,
                              alpha, beta, tau, eps) {}

  r_iae_kalman_filter_t &update(const numeric_vector<M_> &z) override {
    const numeric_vector<M_>     y    = z - this->H_ * this->x_;
    const numeric_matrix<N_, M_> PHT  = this->P_.matmul_T(this->H_);
    const numeric_matrix<M_, M_> S    = this->H_ * PHT + this->R_;
    const numeric_matrix<M_, M_> invS = S.inverse();
    const numeric_matrix<N_, M_> K    = PHT * invS;

    if (!S.is_spd(eps_)) {
      // If S is bad, skip adaptation this step, use normal KF update
      this->x_ += K * y;
      const numeric_matrix<N_, N_> I_KH = numeric_matrix<N_, N_>::identity() - K * this->H_;
      this->P_                          = I_KH * this->P_ * I_KH.transpose() + K * this->R_ * K.transpose();
      this->P_                          = 0.5 * (this->P_ + this->P_.transpose());
      return *this;
    }

    // Mahalanobis distance d = sqrt(y^T S^-1 y)
    const numeric_matrix<M_, 1> y_col = y.as_matrix_col();
    const numeric_matrix<1, 1>  d2m   = y_col.transpose() * invS * y_col;

    const real_t d = sqrt(d2m[0][0] + eps_);
    real_t       w = d <= tau_
                       ? 1.
                       : tau_ / d;  // Huber weight in [0,1]
    if (w < eps_)
      w = eps_;

    // Weighted innovation and gain
    numeric_vector<M_>     y_w   = w * y;
    numeric_matrix<N_, M_> K_eff = w * K;

    // State update
    this->x_ += K_eff * y;

    // Joseph form with consistent gain
    const numeric_matrix<N_, N_> I    = numeric_matrix<N_, N_>::identity();
    const numeric_matrix<N_, N_> I_KH = I - K_eff * this->H_;
    this->P_                          = I_KH * this->P_ * I_KH.transpose() + K_eff * this->R_ * K_eff.transpose();

    // Symmetrize -> Makes P SPD
    this->P_ = 0.5 * (this->P_ + this->P_.transpose());

    // IAE adaptation using robust innovation
    const numeric_matrix<M_, 1>  y_w_col = y_w.as_matrix_col();
    const auto                   ywywT   = y_w_col.matmul_T(y_w_col);  // (w y)(w y)^T = w^2 y y^T
    const numeric_matrix<M_, M_> HPHT    = this->H_ * PHT;

    // Adapt R
    numeric_matrix<M_, M_> R_new = ywywT - HPHT;
    R_new.inplace_project_to_psd(eps_);
    this->R_ = (1. - alpha_) * this->R_ + alpha_ * R_new;

    // Adapt Q
    numeric_matrix<N_, N_> Q_new = K * ywywT * K.transpose();
    Q_new.inplace_project_to_psd(eps_);
    this->Q_ = (1. - beta_) * this->Q_ + beta_ * Q_new;

    return *this;
  }
};


template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
class extended_kalman_filter_t {
private:
  // Note that numeric_matrix<N_, M_> maps from R_^M_ to R_^N_
  static constexpr size_t N_ = StateVectorDimension;        // ALias
  static constexpr size_t M_ = MeasurementVectorDimension;  // Alias
  static constexpr size_t L_ = ControlVectorDimension;      // Alias

public:
  using state_func_t           = numeric_vector<N_> (*)(const numeric_vector<N_> &x, const numeric_vector<L_> &u);
  using state_jacobian_t       = numeric_matrix<N_, N_> (*)(const numeric_vector<N_> &x, const numeric_vector<L_> &u);
  using observation_func_t     = numeric_vector<M_> (*)(const numeric_vector<N_> &x);
  using observation_jacobian_t = numeric_matrix<M_, N_> (*)(const numeric_vector<N_> &x);

protected:
  const state_func_t            f_;   // state-transition model
  const state_jacobian_t        Fj_;  // state-transition Jacobian
  const observation_func_t      h_;   // measurement model
  const observation_jacobian_t  Hj_;  // measurement Jacobian
  const numeric_matrix<N_, N_> &Q_;   // covariance of the process noise
  const numeric_matrix<M_, M_> &R_;   // covariance of the measurement noise
  numeric_vector<N_>            x_;   // state vector
  numeric_matrix<N_, N_>        P_;   // state covariance, self-initialized as Q_

public:
  constexpr extended_kalman_filter_t(
    const state_func_t            f_vec_func,
    const state_jacobian_t        Fj_mat_func,
    const observation_func_t      h_vec_func,
    const observation_jacobian_t  Hj_mat_func,
    const numeric_matrix<N_, N_> &Q_matrix,
    const numeric_matrix<M_, M_> &R_matrix,
    const numeric_vector<N_>     &x_0)
      : f_(f_vec_func), Fj_{Fj_mat_func}, h_{h_vec_func}, Hj_{Hj_mat_func},
        Q_{Q_matrix}, R_{R_matrix}, x_{x_0}, P_{Q_matrix} {}

  constexpr extended_kalman_filter_t(const extended_kalman_filter_t &) = default;

  constexpr extended_kalman_filter_t(extended_kalman_filter_t &&) noexcept = default;

  extended_kalman_filter_t &operator=(const extended_kalman_filter_t &) = default;

  extended_kalman_filter_t &operator=(extended_kalman_filter_t &&) noexcept = default;

  extended_kalman_filter_t &predict(const numeric_vector<L_> &u = {}) {
    x_                        = move(f_(x_, u));
    numeric_matrix<N_, N_> F_ = move(Fj_(x_, u));
    P_                        = move(F_ * P_.matmul_T(F_) + Q_);
    return *this;
  }

  extended_kalman_filter_t &update(const numeric_vector<M_> &z) {
    numeric_vector<M_>     y_      = move(z - h_(x_));
    numeric_matrix<M_, N_> Hjx_    = move(Hj_(x_));
    numeric_matrix<N_, M_> P_Hjx_t = move(P_.matmul_T(Hjx_));
    numeric_matrix<M_, M_> S_      = move(Hjx_ * P_Hjx_t + R_);
    numeric_matrix<N_, M_> K_      = move(P_Hjx_t * S_.inverse());

    x_ += K_ * y_;
    P_ = move((numeric_matrix<N_, N_>::identity() - K_ * Hj_(x_)) * P_);

    return *this;
  }

  extended_kalman_filter_t &operator<<(const numeric_vector<M_> &z) {
    return predict().update(z);
  }

  template<typename... Ts>
  extended_kalman_filter_t &update(Ts... vs) { return update(make_numeric_vector({vs...})); }

  const numeric_vector<N_> &state_vector = x_;

  const real_t &state = x_[0];
};

template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
class adaptive_extended_kalman_filter_t {
private:
  // Note that numeric_matrix<N_, M_> maps from R_^M_ to R_^N_
  static constexpr size_t N_ = StateVectorDimension;        // ALias
  static constexpr size_t M_ = MeasurementVectorDimension;  // Alias
  static constexpr size_t L_ = ControlVectorDimension;      // Alias

public:
  using state_func_t           = numeric_vector<N_> (*)(const numeric_vector<N_> &x, const numeric_vector<L_> &u);
  using state_jacobian_t       = numeric_matrix<N_, N_> (*)(const numeric_vector<N_> &x, const numeric_vector<L_> &u);
  using observation_func_t     = numeric_vector<M_> (*)(const numeric_vector<N_> &x);
  using observation_jacobian_t = numeric_matrix<M_, N_> (*)(const numeric_vector<N_> &x);

protected:
  const state_func_t           f_;      // state-transition model
  const state_jacobian_t       Fj_;     // state-transition Jacobian
  const observation_func_t     h_;      // measurement model
  const observation_jacobian_t Hj_;     // measurement Jacobian
  numeric_matrix<N_, N_>      &Q_;      // covariance of the process noise
  numeric_matrix<M_, M_>      &R_;      // covariance of the measurement noise
  numeric_vector<N_>           x_;      // state vector
  numeric_matrix<N_, N_>       P_;      // state covariance, self-initialized as Q_
  const real_t                 alpha_;  // EMA Smoothing factor for R
  const real_t                 beta_;   // EMA Smoothing factor for Q

public:
  constexpr adaptive_extended_kalman_filter_t(
    const state_func_t           f_vec_func,
    const state_jacobian_t       Fj_mat_func,
    const observation_func_t     h_vec_func,
    const observation_jacobian_t Hj_mat_func,
    numeric_matrix<N_, N_>      &Q_matrix,
    numeric_matrix<M_, M_>      &R_matrix,
    const numeric_vector<N_>    &x_0,
    const real_t                &alpha = 0.1,
    const real_t                &beta  = 0.1)
      : f_(f_vec_func), Fj_{Fj_mat_func}, h_{h_vec_func}, Hj_{Hj_mat_func},
        Q_{Q_matrix}, R_{R_matrix}, x_{x_0}, P_{Q_matrix},
        alpha_{alpha}, beta_{beta} {}

  constexpr adaptive_extended_kalman_filter_t(const adaptive_extended_kalman_filter_t &) = default;

  constexpr adaptive_extended_kalman_filter_t(adaptive_extended_kalman_filter_t &&) noexcept = default;

  adaptive_extended_kalman_filter_t &operator=(const adaptive_extended_kalman_filter_t &) = default;

  adaptive_extended_kalman_filter_t &operator=(adaptive_extended_kalman_filter_t &&) noexcept = default;

  adaptive_extended_kalman_filter_t &predict(const numeric_vector<L_> &u = {}) {
    x_                        = move(f_(x_, u));
    numeric_matrix<N_, N_> F_ = move(Fj_(x_, u));
    P_                        = move(F_ * P_.matmul_T(F_) + Q_);
    return *this;
  }

  adaptive_extended_kalman_filter_t &update(const numeric_vector<M_> &z) {
    numeric_vector<M_>     y_      = move(z - h_(x_));
    numeric_matrix<M_, N_> Hjx_    = move(Hj_(x_));
    numeric_matrix<N_, M_> P_Hjx_t = move(P_.matmul_T(Hjx_));
    numeric_matrix<M_, M_> S_      = move(Hjx_ * P_Hjx_t + R_);
    numeric_matrix<N_, M_> K_      = move(P_Hjx_t * S_.inverse());

    x_ += K_ * y_;
    P_ = move((numeric_matrix<N_, N_>::identity() - K_ * Hj_(x_)) * P_);

    numeric_matrix<M_, 1>  y_mat = y_.as_matrix_col();
    numeric_matrix<M_, M_> y_yT  = y_mat.matmul_T(y_mat);

    adapt_R(y_yT, S_);
    adapt_Q(K_, y_yT);

    return *this;
  }

  adaptive_extended_kalman_filter_t &operator<<(const numeric_vector<M_> &z) {
    return predict().update(z);
  }

  template<typename... Ts>
  adaptive_extended_kalman_filter_t &update(const Ts &...vs) { return update(make_numeric_vector({vs...})); }

  const numeric_vector<N_>     &state_vector = x_;
  const real_t                 &state        = x_[0];
  const numeric_matrix<M_, M_> &R            = R_;
  const numeric_matrix<N_, N_> &Q            = Q_;

private:
  void adapt_R(const numeric_matrix<M_, M_> &y_yT, const numeric_matrix<M_, M_> &S) {
    R_ = (1 - alpha_) * R_ + alpha_ * (y_yT + S);
  }

  void adapt_Q(const numeric_matrix<N_, M_> &K, const numeric_matrix<M_, M_> &y_yT) {
    Q_ = (1 - beta_) * Q_ + beta_ * (K * y_yT * K.transpose());
  }
};

namespace future {
  template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
  class unscented_kalman_filter_t {
  private:
    // Note that numeric_matrix<N_, M_> maps from R_^M_ to R_^N_
    static constexpr size_t N_ = StateVectorDimension;          // ALias
    static constexpr size_t M_ = MeasurementVectorDimension;    // Alias
    static constexpr size_t L_ = ControlVectorDimension;        // Alias
    static constexpr size_t Z_ = 2 * StateVectorDimension + 1;  // Sigma Points
  public:
    typedef numeric_vector<N_> (*state_func_t)(const numeric_vector<N_> &x, const numeric_vector<L_> &u);

    typedef numeric_vector<M_> (*observation_func_t)(const numeric_vector<N_> &x);

  protected:
    const state_func_t            f_;  // state-transition model
    const observation_func_t      h_;  // measurement model
    const numeric_matrix<N_, N_> &Q_;  // covariance of the process noise
    const numeric_matrix<M_, M_> &R_;  // covariance of the measurement noise
    numeric_vector<N_>            x_;  // state vector
    numeric_matrix<N_, N_>        P_;  // state covariance, self-initialized as Q_

  public:
    void predict() {
    }

    void update() {
    }

  protected:
    void cp_sigma() {
    }

    void cp_weight() {
    }
  };
}  // namespace future

// Aliases
template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
using KF =
  kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension>;

template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
using EKF =
  extended_kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension>;

namespace basic {
  class KalmanFilter_1D {
  private:
    real_t m_x;  // Estimated state
    real_t m_P;  // Estimated error covariance
    real_t m_Q;  // Process noise covariance
    real_t m_R;  // Measurement noise covariance
    real_t m_K;  // Kalman gain

  public:
    constexpr KalmanFilter_1D() : KalmanFilter_1D(initial_x, initial_P, initial_noise, initial_noise) {}

    constexpr KalmanFilter_1D(const real_t &initial_x, const real_t &initial_P,
                              const real_t &Q, const real_t &R)
        : m_x(initial_x), m_P(initial_P), m_Q(Q), m_R(R), m_K(0.0) {
    }

    KalmanFilter_1D &predict(const real_t & = 0.0) {
      m_P = m_P + m_Q;
      return *this;
    }

    KalmanFilter_1D &update(const real_t &z) {
      m_K = m_P / (m_P + m_R);
      m_x = m_x + m_K * (z - m_x);
      m_P = (1 - m_K) * m_P;
      return *this;
    }

    KalmanFilter_1D &operator<<(const real_t &z) {
      return predict().update(z);
    }

    [[nodiscard]] constexpr real_t x() const {
      return m_x;
    }

    [[nodiscard]] constexpr real_t P() const {
      return m_P;
    }

    void operator>>(real_t &targ) const {
      targ = x();
    }

    static constexpr real_t initial_x     = 0.0;
    static constexpr real_t initial_P     = 1.0;
    static constexpr real_t initial_noise = 0.1;
  };
}  // namespace basic

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_LINALG_KALMAN_HPP
