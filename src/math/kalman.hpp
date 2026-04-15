#ifndef LIB_XCORE_LINALG_KALMAN_HPP
#define LIB_XCORE_LINALG_KALMAN_HPP

#include "core/ported_std.hpp"
#include "./numeric_vector.hpp"
#include "./numeric_matrix.hpp"

LIB_XCORE_BEGIN_NAMESPACE

/**
 * # Simple Kalman Filter (discrete, linear, time-invariant per step)
 *
 * `kalman_filter_t<N, M, L>` implements the classic discrete Kalman filter:
 *
 *   xₖ   = F xₖ₋₁ + B uₖ + wₖ,       wₖ ~ 𝒩(0, Q)
 *   zₖ   = H xₖ     + vₖ,            vₖ ~ 𝒩(0, R)
 *
 * where:
 *   - x ∈ ℝᴺ is the state vector           (internal)
 *   - u ∈ ℝᴸ is the control input          (optional)
 *   - z ∈ ℝᴹ is the measurement
 *   - F ∈ ℝᴺ×ᴺ is the state transition
 *   - B ∈ ℝᴺ×ᴸ is the control-input model
 *   - H ∈ ℝᴹ×ᴺ is the measurement model
 *   - Q ∈ ℝᴺ×ᴺ is the process-noise covariance (PSD)
 *   - R ∈ ℝᴹ×ᴹ is the measurement-noise covariance (PSD)
 *
 * ## Template parameters
 * - `StateVectorDimension` (N):    length of the state vector `x`
 * - `MeasurementVectorDimension` (M): length of the measurement vector `z`
 * - `ControlVectorDimension` (L):  length of the control vector `u`
 *
 * ## Storage & ownership
 * The filter **holds references** to `F`, `B`, `H`, `Q`, `R`:
 *   - `F_` and `Q_` are non-const references (you may adapt/tune them over time).
 *   - `B_` and `H_` are const references.
 * You (the caller) own the matrices/vectors and must keep them alive as long as the filter exists.
 * The internal state `x_` and covariance `P_` are owned by the filter.
 *
 * ## Dimensions (compile-time checked)
 * - `F`: N×N
 * - `B`: N×L  (if no control, set L=0 or pass a zero matrix and call `predict()` without arguments)
 * - `H`: M×N
 * - `Q`: N×N (symmetric PSD)
 * - `R`: M×M (symmetric PSD)
 * - `x0`: N
 * - `P0`: N×N (symmetric PSD)
 *
 * ## Initialization
 * - Use the 7-argument ctor when you already have a prior covariance `P0`.
 * - The 6-argument ctor initializes `P_` from `Q` (i.e., `P0 = Q`) which is a reasonable
 *   "uninformative" starting point if you lack a prior.
 * - Always ensure `Q`, `R`, and `P0` are symmetric positive semidefinite (PSD). The
 *   implementation re-symmetrizes `P` after predict/update for numerical hygiene.
 *
 * ## Choosing models (F, B, H)
 * 1) **Start from kinematics** that match your state:
 *    - Constant position:       x = [p]
 *    - Constant velocity (1D):  x = [p, v]
 *      F(Δt) = [[1, Δt],
 *               [0,  1 ]]
 *    - Constant acceleration:   x = [p, v, a]
 *      F(Δt) = [[1, Δt, ½Δt²],
 *               [0,  1,    Δt],
 *               [0,  0,      1]]
 *
 * 2) **Control model `B`** maps known inputs (e.g., commanded acceleration) to state delta:
 *    - For CV + accel input: x=[p, v], u=[a]
 *      B(Δt) = [[½Δt²],
 *               [  Δt ]]
 *
 * 3) **Measurement model `H`** maps state to your sensor outputs:
 *    - If a sensor measures position only for x=[p, v]: H = [[1, 0]]
 *    - If it measures position & velocity: H = I₂
 *
 * Keep units consistent across F, B, H, Q, R, x, u, z.
 *
 * ## Choosing noise covariances (Q, R)
 * - **R (measurement noise):** Start from sensor specs.
 *   - If your M sensors are independent, a diagonal R with variances σᵢ² is fine:
 *     R = diag(σ₁², σ₂², …)
 *   - If sensors are correlated, include off-diagonal covariances.
 *
 * - **Q (process noise):** Encodes *model mismatch* (unmodeled accelerations, jerks, drifts).
 *   Practical recipes:
 *   1) **Diagonal heuristic:** Start with small diagonal values in the state components you expect to drift.
 *      Increase until innovations (residuals) look like zero-mean noise and the filter tracks dynamics
 *      without lagging or oversmoothing.
 *   2) **Kinematic spectral density:** For CV model (x=[p, v]), let q be the (scalar) power spectral density
 *      of white acceleration noise. Then with timestep Δt:
 *        Q = q * [[Δt⁴/4, Δt³/2],
 *                  [Δt³/2, Δt²  ]]
 *      For CA model, use the standard continuous-to-discrete mapping to build Q from a white jerk PSD.
 *
 * Rule of thumb: If the filter **lags** real motion → increase Q (or reduce R).
 * If it **chases measurement noise** → decrease Q (or increase R).
 *
 * ## Initial state and covariance (x0, P0)
 * - Set `x0` from your best prior (e.g., first measurement mapped through H⁺).
 * - Set `P0` large if uncertain (e.g., diag with big variances), small if confident.
 * - If unsure, start with `P0 = Q` (via the 6-arg ctor) and refine later.
 *
 * ## Runtime adaptation
 * - You may tune `F` and `Q` over time (e.g., piecewise-constant Δt or mode changes) since the
 *   filter holds non-const references to them.
 * - `predict()` with no argument uses zero control; `predict(u)` uses your control input.
 *
 * ## Numerical tips
 * - Keep `Q` and `R` symmetric PSD. Small positive epsilons on diagonals help when values are near zero.
 * - Ensure Δt is well-scaled; extremely large or tiny Δt can harm conditioning.
 * - Joseph-form update is used to keep `P` PSD under finite precision. Post-symmetrization further stabilizes it.
 *
 * ## Example (1D position+velocity, position-only measurements)
 *   N=2, M=1, L=1 (accel control optional)
 *
 *   // State x = [p, v], u = [a], z = [p_meas]
 *   real_t dt = 0.01;
 *   numeric_matrix<2,2> F = {{1, dt},
 *                            {0,  1}};
 *   numeric_matrix<2,1> B = {{0.5*dt*dt},
 *                            {dt}};
 *   numeric_matrix<1,2> H = {{1, 0}};
 *
 *   // Noises
 *   real_t sigma_z = 0.05;                  // position sensor std (m)
 *   numeric_matrix<1,1> R = {{sigma_z*sigma_z}};
 *
 *   real_t q = 0.5;                          // accel PSD (m^2/s^3), tune this
 *   numeric_matrix<2,2> Q = {{q*dt*dt*dt*dt/4, q*dt*dt*dt/2},
 *                            {q*dt*dt*dt/2,    q*dt*dt}};
 *
 *   // Initial conditions
 *   numeric_vector<2> x0 = {0.0, 0.0};
 *   numeric_matrix<2,2> P0 = {{1.0, 0.0},
 *                             {0.0, 1.0}};   // large-ish uncertainty
 *
 *   // Construct the filter
 *   kalman_filter_t<2,1,1> kf(F, B, H, Q, R, x0, P0);
 *
 *   // Usage per cycle:
 *   // kf.predict(u);  // or kf.predict();
 *   // kf.update(z);
 *
 * ## Example (2D CV, no control, measuring position x/y)
 *   N=4, M=2, L=0; x=[px, py, vx, vy], z=[px, py]
 *   F = blockdiag( [[1,dt],[0,1]], [[1,dt],[0,1]] );
 *   H = [[1,0,0,0],
 *        [0,1,0,0]];
 *   R = diag(σ_px², σ_py²);
 *   Q = blockdiag(Q_cv(dt,qx), Q_cv(dt,qy));
 *
 * ## Gotchas
 * - If you pass near-zero variances in `R` or `Q`, ill-conditioning may appear.
 * - Mismatched units or shapes will cause compile-time/type errors or poor estimates.
 * - If your sensor model is nonlinear, this linear filter will be biased—use EKF/UKF variants.
 */
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
   * @param x0 initial state vector
   */
  constexpr kalman_filter_t(
    numeric_matrix<N_, N_>       &F_matrix,
    const numeric_matrix<N_, L_> &B_matrix,
    const numeric_matrix<M_, N_> &H_matrix,
    numeric_matrix<N_, N_>       &Q_matrix,
    numeric_matrix<M_, M_>       &R_matrix,
    const numeric_vector<N_>     &x0)
      : kalman_filter_t(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x0, Q_matrix) {}

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

  const numeric_vector<N_>     &state_vector() { return x_; }
  const real_t                 &state() { return x_[0]; }
  const numeric_matrix<M_, M_> &R() { return R_; }
  const numeric_matrix<N_, N_> &Q() { return Q_; }
};

/**
 * # IAE (Innovation-Adaptive) Kalman Filter
 *
 * `iae_kalman_filter_t<N, M, L>` extends the linear discrete Kalman filter with
 * **online estimation of R and Q** from the innovation sequence (Sage-Husa algorithm).
 * No Jacobians or sigma points — same linear model as `kalman_filter_t`.
 *
 * ## Model
 *   xₖ = F xₖ₋₁ + B uₖ + wₖ,   wₖ ~ 𝒩(0, Q)
 *   zₖ = H xₖ     + vₖ,        vₖ ~ 𝒩(0, R)
 *
 * ## Update equations (after the base Joseph-form P update)
 *   yₖ    = zₖ − H x̂ₖ|ₖ₋₁              innovation (uses prior state)
 *   HPHᵀ  = H Pₖ|ₖ₋₁ Hᵀ               prior output covariance
 *
 *   R_new = project_psd( yₖ yₖᵀ − HPHᵀ )
 *   R     ← (1−α) R + α R_new           EMA blend, then symmetrize
 *
 *   Q_new = project_psd( K yₖ yₖᵀ Kᵀ )
 *   Q     ← (1−β) Q + β Q_new           EMA blend, then symmetrize
 *
 *   In expectation: E[yₖ yₖᵀ] = HPHᵀ + R, so R_new → R as the filter converges.
 *   Q_new approximates the covariance of the state correction K·yₖ.
 *
 * ## IAE vs R-IAE — choose based on your noise environment
 *
 *   | Noise character                               | Use          |
 *   |-----------------------------------------------|--------------|
 *   | Gaussian, stationary, well-known              | Simple KF    |
 *   | Unknown / slowly drifting, no outliers        | IAE          |
 *   | Unknown / drifting + occasional outliers      | R-IAE        |
 *   | Heavy-tailed (GPS multipath, sonar, RF)       | R-IAE        |
 *   | Hard CPU budget, no sqrt needed               | IAE          |
 *
 *   IAE and R-IAE share the same α/β semantics. R-IAE adds a Huber weight that
 *   down-scales the gain and innovation when the Mahalanobis distance exceeds τ,
 *   so a single spike cannot corrupt R or Q. IAE has no such protection — one
 *   large outlier can push R_new negative, which `project_psd` clips to ε but
 *   may still transiently distort the estimate.
 *
 * ## Template parameters
 * - `StateVectorDimension` (N):        length of the state vector x
 * - `MeasurementVectorDimension` (M):  length of the measurement vector z
 * - `ControlVectorDimension` (L):      length of the control vector u
 *
 * ## Storage & ownership
 * Inherits `kalman_filter_t`. Holds **references** to F, B, H, Q, R (caller owns them).
 * Q and R are modified at runtime by adaptation. F may be updated between steps (e.g., variable Δt).
 *
 * ## Parameters: α (alpha) and β (beta)
 *
 * Both are EMA smoothing factors in [0, 1).
 * Effective memory window ≈ 1/α samples; half-life = ln(0.5) / ln(1−α).
 *
 *   α = 0.05  →  ~20-sample memory,  half-life ≈ 14 steps
 *   α = 0.10  →  ~10-sample memory,  half-life ≈  7 steps
 *   α = 0.20  →   ~5-sample memory,  half-life ≈  3 steps
 *
 * ### α — controls how fast R tracks changing measurement noise
 *
 *   α = 0          Freeze R (no adaptation). Equivalent to Simple KF for R.
 *   0 < α ≤ 0.02   Extremely slow; R barely moves. Useful only for very long runs.
 *   0.02 – 0.05    Slow drift. Good default for stable sensors (IMU bias, thermistor).
 *   0.05 – 0.10    Moderate drift. Good default for most embedded sensors.       ← start here
 *   0.10 – 0.20    Fast drift. Use when noise statistics change within seconds.
 *   α > 0.20       R becomes too noisy sample-to-sample. Generally avoid.
 *
 *   Rule: if NIS (normalized innovation squared, yᵀS⁻¹y) stays consistently above M,
 *   R is underestimated → increase α. If NIS stays below M, R is overestimated → decrease α.
 *
 * ### β — controls how fast Q tracks changing process noise
 *
 *   β = 0          Freeze Q. Enable only after R adaptation is stable.
 *   0.005 – 0.01   Conservative. Recommended starting point. Resists over-inflation.  ← start here
 *   0.01  – 0.03   Moderate. Adapt to regime changes (stop/go, terrain change).
 *   0.03  – 0.05   Aggressive. Use only if the filter lags obviously in fast maneuvers.
 *   β > 0.05       Q inflates quickly → P grows → gains stay high → estimates jitter.
 *
 *   Keep β ≤ α. Q affects P growth in the predict step, which couples back into gain
 *   calculation and stability. Over-tuning β is harder to recover from than over-tuning α.
 *
 *   Rule: if the filter lags real motion despite correct R → increase β.
 *         if estimates are jittery after noise events → decrease β.
 *
 * ## Tuning workflow
 * 1) Start with a stable Simple KF (hand-tuned Q₀, R₀).
 * 2) Switch to IAE. Set α = 0.05, β = 0. Run and monitor NIS = yᵀS⁻¹y:
 *    - NIS ≈ M (dof) over time → R tracking is sane.
 *    - NIS ≫ M → increase α (R too small or adapts too slowly).
 *    - NIS ≪ M → decrease α (R overshoots).
 * 3) Enable β = 0.01. Watch for lag vs jitter:
 *    - Filter lags → increase β by 0.005 at a time.
 *    - Estimates jitter → decrease β or check that α is not too small.
 * 4) If outliers corrupt the estimates, switch to R-IAE instead.
 *
 * ## Initialization
 * - Seed Q₀, R₀ from your best prior (sensor spec, model-derived PSD).
 * - A rough seed is fine; adaptation corrects it within 1/α steps.
 * - The 6-arg ctor sets P₀ = Q₀ as a neutral start if you have no prior covariance.
 *
 * ## Numerical hygiene (already applied internally)
 * After each EMA blend, R and Q are PSD-projected (negative eigenvalues clipped to ε)
 * and symmetrized. You may still want to add domain-specific diagonal floors/ceilings
 * on your Q and R matrices to prevent physically impossible values (e.g., negative
 * position variance, R dropping below sensor resolution²).
 *
 * ## Gotchas
 * - **Model bias** (wrong H or F): R inflates to absorb residuals — fix the model first.
 * - **Variable Δt**: recompute F (and any kinematically derived Q₀) between steps.
 *   Adaptation trims the residual mismatch but cannot compensate a wrong transition model.
 * - **Cold start**: NIS is unreliable for the first ~1/α steps while R is converging.
 *   Treat early estimates with lower confidence.
 * - **α = β = 0**: reduces to a Simple KF with the initial Q₀, R₀ — useful for debugging.
 * - This class is `final`; override points are in `kalman_filter_t` if you need custom behaviour.
 */

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
   * @param x0 initial state vector
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
    const real_t                 &alpha = 0.1,
    const real_t                 &beta  = 0.1)
      : iae_kalman_filter_t(F_matrix, B_matrix, H_matrix, Q_matrix, R_matrix, x0, Q_matrix, alpha, beta) {}

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
    numeric_matrix<M_, M_>       R_new = y_yT - HPHT;
    R_new.inplace_project_to_psd();
    this->R_ = (1. - alpha_) * this->R_ + alpha_ * R_new;
    this->R_ = 0.5 * (this->R_ + this->R_.transpose());

    // Adapt Q
    numeric_matrix<N_, N_> Q_new = K * y_yT * K.transpose();
    Q_new.inplace_project_to_psd();
    this->Q_ = (1 - beta_) * this->Q_ + beta_ * Q_new;
    this->Q_ = 0.5 * (this->Q_ + this->Q_.transpose());

    return *this;
  }
};

/**
 * # Robust IAE Kalman Filter (R-IAE)
 *
 * `r_iae_kalman_filter_t<N, M, L>` combines the IAE adaptive filter with
 * **Huber M-estimation**: the gain and innovation are down-weighted whenever the
 * Mahalanobis distance of the innovation exceeds a threshold τ. This prevents
 * outliers from corrupting the state estimate, the covariance, and the adapted Q/R.
 *
 * ## Model
 *   xₖ = F xₖ₋₁ + B uₖ + wₖ,   wₖ ~ 𝒩(0, Q)
 *   zₖ = H xₖ     + vₖ,        vₖ ~ 𝒩(0, R)
 *
 * ## Update equations
 *   yₖ   = zₖ − H x̂ₖ|ₖ₋₁              innovation
 *   S    = H Pₖ|ₖ₋₁ Hᵀ + R             innovation covariance
 *   d    = √( yₖᵀ S⁻¹ yₖ + ε )         Mahalanobis distance (ε for numerical safety)
 *
 *   Huber weight:
 *     w = 1          if d ≤ τ            (nominal; full update)
 *     w = τ / d      if d > τ            (outlier; scale down)
 *
 *   K_eff = w · K    where K = Pₖ|ₖ₋₁ Hᵀ S⁻¹
 *   y_w   = w · y
 *
 *   x̂ₖ|ₖ = x̂ₖ|ₖ₋₁ + K_eff · yₖ        state update (gains scaled, raw y)
 *   Pₖ|ₖ = (I−K_eff H) P (I−K_eff H)ᵀ + K_eff R K_effᵀ   Joseph form
 *
 *   R_new = project_psd( y_w y_wᵀ − HPHᵀ )
 *   R_old = R                                              saved before EMA
 *   R     ← (1−α) R + α R_new,  then symmetrize
 *
 *   Q_new = project_psd( K_eff y_w y_wᵀ K_effᵀ − K_eff R_old K_effᵀ )
 *   Q     ← (1−β) Q + β Q_new
 *
 *   If S is not SPD (numerical failure), the step falls back to a plain KF
 *   update without adaptation and returns immediately.
 *
 * ## IAE vs R-IAE — key differences
 *
 *   Both classes adapt R and Q every step using the same α/β EMA logic.
 *   The only difference is the Huber weight w:
 *
 *     IAE:    w = 1 always          (no outlier protection)
 *     R-IAE:  w ∈ (0, 1]           (outliers are down-weighted by τ/d)
 *
 *   Consequences:
 *   - A spike with d = 10σ gets w = τ/10 ≈ 0.3 at τ=3, so its effect on x, P, R, Q
 *     is reduced by ~70% compared to IAE.
 *   - R-IAE costs one extra matrix inversion (S⁻¹ is reused) and a scalar sqrt per step.
 *   - Use IAE when sensors are clean; use R-IAE whenever spikes are possible.
 *
 *   See `iae_kalman_filter_t` for the α/β decision table and tuning workflow —
 *   everything there applies to R-IAE. The only additional parameter is τ.
 *
 * ## Template parameters
 * - `StateVectorDimension` (N):        length of the state vector x
 * - `MeasurementVectorDimension` (M):  length of the measurement vector z
 * - `ControlVectorDimension` (L):      length of the control vector u
 *
 * ## Storage & ownership
 * Inherits `kalman_filter_t`. Holds **references** to F, B, H, Q, R (caller owns them).
 * Q and R are modified at runtime by adaptation. F may be updated between steps.
 *
 * ## Parameter τ (tau) — Huber threshold
 *
 * τ is in Mahalanobis units: d = √(yᵀ S⁻¹ y) ~ √(χ²_M) under nominal Gaussian noise.
 *
 * Under a correct model the distribution of d depends on M (measurement dimension):
 *   M=1:  d ~ half-normal; 99.7% of nominal samples have d < 3.0
 *   M=2:  d ~ Rayleigh;    99.7% of nominal samples have d < 3.43
 *   M=3:  99.7% have d < 3.74    (grows slowly with M)
 *
 * A conservative τ passes more inliers unweighted; an aggressive τ rejects more.
 *
 *   τ = 2.5    Aggressive. Rejects ~1–2% of true inliers (M=1). Use with frequent spikes.
 *   τ = 3.0    Balanced. Default. Rejects ~0.3% of inliers at M=1.                 ← start here
 *   τ = 3.5    Conservative. Nearly no inlier rejection. Use when outliers are rare.
 *   τ > 5.0    Barely different from IAE. Robustness effectively disabled.
 *
 *   For M > 1, consider scaling τ by √(χ²_M,0.997) / √(χ²_1,0.997) ≈ √(M/1) as a
 *   rough correction to keep the false-rejection rate constant across dimensions.
 *
 * ## Parameter ε (eps) — numerical floor
 *
 * Used in the Mahalanobis sqrt (avoids √0) and in the SPD check for S.
 * Does not affect filter accuracy unless matrices are near-singular.
 *
 *   ε = 1e−12   Default. Correct for double-precision, SI-scaled states (metres, rad/s).
 *   ε = 1e−9    Use if your matrix entries are tiny (µm, nrad) to avoid false SPD failures.
 *   ε = 1e−6    Use with single-precision (float) arithmetic.
 *
 * ## Quick-start presets
 *
 *   Noise character                  α      β      τ      ε
 *   ───────────────────────────────  ─────  ─────  ─────  ──────
 *   Slow drift, rare spikes          0.05   0.01   3.0    1e-12   ← safe default
 *   Moderate drift, frequent spikes  0.10   0.01   2.7    1e-12
 *   Fast regime changes + spikes     0.10   0.03   3.0    1e-12
 *   Adapt R only (freeze Q)          0.05   0.00   3.0    1e-12
 *
 * ## Behavior cues
 *   - **Outliers still corrupt the state:**  lower τ; or increase α so R rises to absorb them.
 *   - **Valid maneuvers are rejected:**      raise τ slightly (was set too aggressively).
 *   - **Filter lags real motion:**           increase β; or slightly raise τ (more of the maneuver passes).
 *   - **Estimates jitter:**                  decrease β; or lower α if R is oscillating.
 *   - **P collapses (overconfident):**       increase β or add a diagonal floor to Q.
 *   - **R grows without bound:**             add a domain-specific ceiling on diagonal entries of R.
 *
 * ## Monitoring
 *   - **NIS** = yᵀ S⁻¹ y should average ≈ M under a correctly specified model.
 *   - **trace(R)**, **trace(Q)** over time: a persistent upward trend means the model
 *     (H, F, Δt) is wrong or the noise source has changed fundamentally.
 *   - **w histogram**: if most steps have w ≪ 1, τ is too small or the model is biased.
 *
 * ## Gotchas
 * - **Model bias** (wrong H or F): R inflates to absorb residuals even without outliers. Fix the model.
 * - **τ too small + α large**: most updates are heavily down-weighted, gains stay high,
 *   R inflates persistently, and the filter may stall.
 * - **Variable Δt**: recompute F (and any kinematically derived Q₀) between steps.
 * - This class is `final`; override points are in `kalman_filter_t` if you need custom behaviour.
 */
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
    // Innovation and S using PRIOR P (P_)
    const numeric_vector<M_>     y   = z - this->H_ * this->x_;
    const numeric_matrix<N_, M_> PHT = this->P_.matmul_T(this->H_);
    const numeric_matrix<M_, M_> S   = this->H_ * PHT + this->R_;

    // 1) Check SPD before inverting
    if (!S.is_spd(eps_)) {
      // Fallback: plain KF update without adaptation
      const numeric_matrix<M_, M_> S_reg      = S + numeric_matrix<M_, M_>::identity() * eps_;
      const numeric_matrix<N_, M_> K_fallback = PHT * S_reg.inverse();
      this->x_ += K_fallback * y;
      const auto I    = numeric_matrix<N_, N_>::identity();
      const auto I_KH = I - K_fallback * this->H_;
      this->P_        = I_KH * this->P_ * I_KH.transpose() + K_fallback * this->R_ * K_fallback.transpose();
      this->P_        = 0.5 * (this->P_ + this->P_.transpose());
      return *this;
    }

    const numeric_matrix<M_, M_> invS = S.inverse();
    const numeric_matrix<N_, M_> K    = PHT * invS;

    // Mahalanobis distance
    const numeric_matrix<M_, 1> y_col = y.as_matrix_col();
    const numeric_matrix<1, 1>  d2m   = y_col.transpose() * invS * y_col;
    const real_t                d     = ::std::sqrt(d2m[0][0] + eps_);

    // Huber weight in (0,1]
    real_t w = d <= tau_ ? 1. : tau_ / (d + eps_);
    if (w < eps_) w = eps_;

    // Weighted innovation and effective gain
    const numeric_vector<M_>     y_w   = w * y;
    const numeric_matrix<N_, M_> K_eff = w * K;

    // State update
    this->x_ += K_eff * y;

    // Joseph form with K_eff (consistency with robust weighting)
    const auto I    = numeric_matrix<N_, N_>::identity();
    const auto I_KH = I - K_eff * this->H_;
    this->P_        = I_KH * this->P_ * I_KH.transpose() + K_eff * this->R_ * K_eff.transpose();
    this->P_        = 0.5 * (this->P_ + this->P_.transpose());

    // IAE adaptation using robust innovation
    const numeric_matrix<M_, 1>  y_w_col = y_w.as_matrix_col();
    const auto                   ywywT   = y_w_col.matmul_T(y_w_col);  // (w y)(w y)^T
    const numeric_matrix<M_, M_> HPHT    = this->H_ * PHT;             // uses PRIOR P

    // 2) Adapt R (PSD-projected)
    numeric_matrix<M_, M_> R_new = ywywT - HPHT;
    R_new.inplace_project_to_psd(eps_);
    const numeric_matrix<M_, M_> R_prior = this->R_;
    this->R_                             = (1. - alpha_) * this->R_ + alpha_ * R_new;
    this->R_                             = 0.5 * (this->R_ + this->R_.transpose());

    // 3) Adapt Q using K_eff for consistency
    numeric_matrix<N_, N_> Q_new = K_eff * ywywT * K_eff.transpose();

    // Bias correction uses prior R for self-consistency within this step
    Q_new -= K_eff * R_prior * K_eff.transpose();

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
    numeric_matrix<N_, N_> F_ = Fj_(x_, u);
    x_                        = f_(x_, u);
    P_                        = F_ * P_.matmul_T(F_) + Q_;
    return *this;
  }

  extended_kalman_filter_t &update(const numeric_vector<M_> &z) {
    numeric_vector<M_>     y_      = move(z - h_(x_));
    numeric_matrix<M_, N_> Hjx_    = move(Hj_(x_));
    numeric_matrix<N_, M_> P_Hjx_t = move(P_.matmul_T(Hjx_));
    numeric_matrix<M_, M_> S_      = move(Hjx_ * P_Hjx_t + R_);
    numeric_matrix<N_, M_> K_      = move(P_Hjx_t * S_.inverse());

    x_ += K_ * y_;
    const numeric_matrix<N_, N_> I_KH = numeric_matrix<N_, N_>::identity() - K_ * Hjx_;
    P_                                = I_KH * P_ * I_KH.transpose() + K_ * R_ * K_.transpose();
    P_                                = 0.5 * (P_ + P_.transpose());

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
    numeric_matrix<N_, N_> F_ = Fj_(x_, u);
    x_                        = f_(x_, u);
    P_                        = F_ * P_.matmul_T(F_) + Q_;
    return *this;
  }

  adaptive_extended_kalman_filter_t &update(const numeric_vector<M_> &z) {
    numeric_vector<M_>           y_      = move(z - h_(x_));
    numeric_matrix<M_, N_>       Hjx_    = move(Hj_(x_));
    numeric_matrix<N_, M_>       P_Hjx_t = move(P_.matmul_T(Hjx_));
    const numeric_matrix<M_, M_> HPHT    = Hjx_ * P_Hjx_t;
    numeric_matrix<M_, M_>       S_      = move(HPHT + R_);
    numeric_matrix<N_, M_>       K_      = move(P_Hjx_t * S_.inverse());

    x_ += K_ * y_;
    const numeric_matrix<N_, N_> I_KH = numeric_matrix<N_, N_>::identity() - K_ * Hjx_;
    P_                                = I_KH * P_ * I_KH.transpose() + K_ * R_ * K_.transpose();
    P_                                = 0.5 * (P_ + P_.transpose());

    numeric_matrix<M_, 1>  y_mat = y_.as_matrix_col();
    numeric_matrix<M_, M_> y_yT  = y_mat.matmul_T(y_mat);

    adapt_R(y_yT, HPHT);
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
  void adapt_R(const numeric_matrix<M_, M_> &y_yT, const numeric_matrix<M_, M_> &HPHT) {
    R_ = (1 - alpha_) * R_ + alpha_ * (y_yT - HPHT);
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

LIB_XCORE_END_NAMESPACE

// template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
// class KF_Simple : LIB_XCORE_NAMESPACE::kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension> {
// public:
//   KF_Simple()
// };

#endif  //LIB_XCORE_LINALG_KALMAN_HPP
