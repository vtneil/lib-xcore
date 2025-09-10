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
 *   - `B_` and `H_` and `R_` are const references.
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
    const numeric_matrix<N_, N_> &Q_matrix,
    const numeric_matrix<M_, M_> &R_matrix,
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
 * `iae_kalman_filter_t<N, M, L>` extends the linear discrete Kalman filter by
 * **online adaptation of R and Q** using the innovation sequence yₖ = zₖ − H x̂ₖ|ₖ₋₁.
 *
 * Base model (same as Simple KF):
 *   xₖ   = F xₖ₋₁ + B uₖ + wₖ,   wₖ ~ 𝒩(0, Q)
 *   zₖ   = H xₖ     + vₖ,        vₖ ~ 𝒩(0, R)
 *
 * This class updates R and Q after each measurement with EMA (exponential moving average):
 *   yₖyₖᵀ  = innovation outer product
 *   R ← (1−α) R + α ( yₖyₖᵀ − H Pₖ|ₖ₋₁ Hᵀ )         // target ≈ S − HPHᵀ = R (in expectation)
 *   Q ← (1−β) Q + β ( K yₖyₖᵀ Kᵀ )                  // Joseph-consistent process inflation
 *
 * ## Template parameters
 * - `StateVectorDimension` (N): state length
 * - `MeasurementVectorDimension` (M): measurement length
 * - `ControlVectorDimension` (L): control length
 *
 * ## Storage & ownership
 * Inherits `kalman_filter_t` and thus **holds references** to `F`, `B`, `H`, `Q`, `R`.
 * - `F_`, `Q_`, `R_` are updated at runtime (Q/R by adaptation, F optionally by you).
 * - Caller owns matrices/vectors; keep them alive while the filter exists.
 *
 * ## When to use IAE-KF
 * - Sensor noise statistics drift over time (R unknown/variable).
 * - Process uncertainty varies with regime (e.g., stop/go, rough/steady motion).
 * - You want a self-tuning filter without hand-retuning Q/R mid-mission.
 *
 * ## Choosing α (alpha) and β (beta)
 * α, β ∈ [0, 1). They are EMA smoothing factors (higher = faster adaptation).
 *
 * - **α (R adaptation)**
 *   - Typical: `0.02 … 0.2`
 *   - Larger α tracks rapidly changing sensor noise but can make R too jumpy.
 *   - Effective memory ~ `1/α` samples; half-life ≈ `ln(0.5)/ln(1−α)`.
 *   - Start with `α = 0.05` (slowly varying sensors) or `α = 0.1` (noisier/uncertain sensors).
 *
 * - **β (Q adaptation)**
 *   - Typical: `0.005 … 0.05` (often **smaller** than α).
 *   - Q inflation affects dynamics and stability more; adapt conservatively.
 *   - Start with `β = 0.01`. Increase if the filter **lags** true motion in regime changes.
 *
 * - **Disable adaptation**: set `α = 0` and/or `β = 0` to freeze R and/or Q.
 *
 * ## Initialization (same shapes as Simple KF)
 * - Provide good `F, B, H` from your kinematic/sensor model.
 * - Seed `R` from sensor specs; seed `Q` from model mismatch (e.g., CV: accel PSD mapping).
 * - `P0`: if unknown, the 6-arg ctor uses `P0 = Q` (reasonable neutral start).
 *
 * ## Tuning workflow
 * 1) Start with a working Simple KF (fixed Q, R) that is roughly stable.
 * 2) Enable **R adaptation first** (α>0, β=0). Check innovations:
 *    - If **Normalized Innovation Squared (NIS)** ≈ χ² mean for M dof over time, R tracking is sane.
 *    - If NIS ≫ target → R too small or α too small; if NIS ≪ target → R too large or α too large.
 * 3) Enable **Q adaptation** (β>0) to reduce lag during regime changes.
 *    - If the filter **chases noise**, reduce β (or increase α so R grows instead).
 *    - If it **lags real motion**, increase β slightly.
 *
 * ## Numerical safety & PSD hygiene
 * The raw updates can yield non-PSD Q/R if data are sparse or inconsistent.
 * Recommended safeguards:
 * - **Symmetrize** after each update: `A ← 0.5 (A + Aᵀ)`.
 * - **Clamp diagonals**: `Aᵢᵢ = max(Aᵢᵢ, eps)` with small `eps` (e.g., 1e−12 in your units).
 * - Optionally **project to PSD** (eigen clip): replace negative eigenvalues with `eps`.
 * - Bound Q and R within reasonable min/max envelopes to avoid blow-ups/deflation:
 *   `A ← clip(A, A_min, A_max)` (elementwise or via spectral norms).
 *
 * ## Behavior cues (what to tweak)
 * - **Track lags behind motion** → increase β (Q inflates faster) or reduce R via larger α.
 * - **Estimates jitter with noisy sensors** → decrease β or increase α (let R rise).
 * - **Covariance collapses unrealistically** (overconfident P) → increase β or floor Q/R.
 * - **Divergence after outliers** → larger α lets R absorb spikes; add R upper bounds.
 *
 * ## Dimensions & units
 * - Keep units consistent across F, B, H, Q, R, x, u, z.
 * - Innovation outer product `y yᵀ` has units of measurement²; the Q update scales via `K`.
 *
 * ## Complexity
 * Same order as Simple KF per step; extra cost is a few matrix multiplies for `y yᵀ`, `HPHᵀ`,
 * and `K y yᵀ Kᵀ`.
 *
 * ## Example starting points
 * - Start from your Simple KF’s F/B/H/Q/R.
 * - Choose `α = 0.05`, `β = 0.01`.
 * - Add symmetrization and diagonal floors after adaptation:
 *     R ← 0.5(R + Rᵀ);  Rᵢᵢ ← max(Rᵢᵢ, eps)
 *     Q ← 0.5(Q + Qᵀ);  Qᵢᵢ ← max(Qᵢᵢ, eps)
 *
 * ## Gotchas
 * - Very small α, β → no meaningful adaptation; very large values → noisy/unstable covariances.
 * - If measurements are biased (model error in H), R may inflate to cover bias; fix H first.
 * - If Δt varies, recompute F (and model-derived Q) accordingly; adaptation then trims residual mismatch.
 *
 * ## Notes
 * - This class is `final`: override points are in the base if you need custom predict/update.
 * - Setting `α=β=0` reduces behavior to the base KF (with your initial Q, R).
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
    this->R_                          = (1. - alpha_) * this->R_ + alpha_ * (y_yT - HPHT);

    // Adapt Q
    this->Q_ = (1 - beta_) * this->Q_ + beta_ * (K * y_yT * K.transpose());

    return *this;
  }
};

/**
 * # Robust IAE Kalman Filter (R-IAE)
 *
 * `r_iae_kalman_filter_t<N, M, L>` augments the Simple KF with:
 *  1) **Robustified update** via Huber weighting on the **Mahalanobis** innovation.
 *  2) **Innovation-Adaptive Estimation (IAE)** of `R` and `Q` using an EMA.
 *
 * Base model (discrete, linear):
 *   xₖ   = F xₖ₋₁ + B uₖ + wₖ,   wₖ ~ 𝒩(0, Q)
 *   zₖ   = H xₖ     + vₖ,        vₖ ~ 𝒩(0, R)
 *
 * Robust step:
 *   yₖ  = zₖ − H x̂ₖ|ₖ₋₁
 *   S   = H Pₖ|ₖ₋₁ Hᵀ + R,  d² = yₖᵀ S⁻¹ yₖ
 *   Huber weight   w = 1                     if d ≤ τ
 *                         τ / d              if d > τ
 *   Use `K_eff = w K` and `y_w = w y` in the Joseph update.
 *
 * IAE step (with robust innovation):
 *   R ← (1−α) R + α ( (y_w y_wᵀ) − H Pₖ|ₖ₋₁ Hᵀ )   // targets the measurement noise part
 *   Q ← (1−β) Q + β ( K (y_w y_wᵀ) Kᵀ )            // process inflation consistent with gain
 * (Class applies PSD projection + symmetrization for numerical hygiene.)
 *
 * ## Template parameters
 * - `StateVectorDimension` (N): state length
 * - `MeasurementVectorDimension` (M): measurement length
 * - `ControlVectorDimension` (L): control length
 *
 * ## Storage & ownership
 * Inherits `kalman_filter_t`; holds **references** to `F`, `B`, `H`, `Q`, `R` (caller owns them).
 * `Q` and `R` are **modified at runtime** by adaptation; `F` may be changed by you between steps.
 *
 * ## When to pick R-IAE (vs Simple/IAE)
 * - Measurements contain **outliers/heavy tails** (spikes, dropouts, multipath).
 * - Noise statistics **drift over time** (need adaptation) *and* you want outlier resilience.
 * - You need a drop-in robustifier without re-deriving sensor models.
 *
 * ## Choosing parameters
 * - **α (alpha)** — EMA smoothing for `R` (0 ≤ α < 1)
 *   - Typical: `0.02 … 0.2`. Faster `R` tracking with larger α; too large → jittery `R`.
 *   - Start: `α = 0.05` (slow drift) or `0.1` (noisier/uncertain sensors).
 *
 * - **β (beta)** — EMA smoothing for `Q` (0 ≤ β < 1)
 *   - Typical: `0.005 … 0.05`. Often **smaller than α** (affects dynamics/stability).
 *   - Start: `β = 0.01`. Increase if filter **lags** genuine regime changes.
 *
 * - **τ (tau)** — Huber threshold in **Mahalanobis σ-units**
 *   - Interpreted on √(χ²_M) scale; common: `τ = 2.5 … 3.5`. Default `3.0`.
 *   - Smaller τ → stronger outlier rejection (more conservative updates).
 *   - Larger τ → closer to standard KF (less robustification).
 *
 * - **ε (eps)** — tiny positive floor (stability)
 *   - Used for SPD checks and divisions; pick relative to your scale.
 *   - Typical: `1e−12` for well-scaled SI units; up to `1e−9` if matrices are tiny/ill-conditioned.
 *
 * ### Quick presets
 * - **Mild outliers, slow drift:**  α=0.05, β=0.01, τ=3.0, ε=1e−12
 * - **Frequent spikes:**           α=0.1,  β=0.01, τ=2.7, ε=1e−12 (consider upper bounds on R)
 * - **Aggressive agility:**        α=0.15, β=0.03, τ=3.2, ε=1e−12 (watch for jitter)
 * - **Adapt R only:**              α>0,    β=0
 *
 * ## Behavior cues & what to tweak
 * - **Outliers still jerk the state:** lower τ (stronger down-weighting), increase α (let R rise).
 * - **Filter lags real maneuvers:**   increase β (inflate Q faster), or slightly increase τ.
 * - **Estimate jitters on noise:**    decrease β (less Q inflation), or increase τ/α.
 * - **P becomes overconfident:**      increase β or floor/clip Q; verify PSD projection is on.
 *
 * ## Numerical and safety notes
 * - Class **checks S SPD**; if not SPD, it falls back to a normal KF update without adaptation.
 * - After each step, `P` is symmetrized; `Q`/`R` updates are PSD-projected and symmetrized.
 * - Consider **diagonal floors/ceilings** for `Q` and `R` in your domain (e.g., min/max sensor variances).
 * - Keep units consistent; Mahalanobis distance already accounts for scaling via `S⁻¹`.
 *
 * ## Monitoring (recommended)
 * - Track **NIS**:  eₖ = yₖᵀ S⁻¹ yₖ  (should hover near χ²_M mean under nominal conditions).
 * - Watch `trace(R)` and `trace(Q)` over time; unexpected drifts often signal model mismatch (H, F, Δt).
 *
 * ## Complexity
 * Near Simple/IAE cost; extra S⁻¹ and Huber weighting are O(M³) dominated by the usual inversion.
 *
 * ## Gotchas
 * - If your **model is biased** (e.g., wrong H), R may inflate persistently to hide bias—fix the model.
 * - Excessively small τ with large α can starve updates (w too small) and stall the filter.
 * - Variable Δt: update F (and any model-based Q) accordingly; R-IAE then trims residual mismatch.
 *
 * ## Minimal usage
 * - Start from a working Simple KF setup (F, B, H, Q, R, x0, P0).
 * - Swap to `r_iae_kalman_filter_t` with: `α = 0.05`, `β = 0.01`, `τ = 3.0`, `ε = 1e−12`.
 * - Iterate: lower τ if outliers still leak; adjust α/β using the behavior cues above.
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

LIB_XCORE_END_NAMESPACE

// template<size_t StateVectorDimension, size_t MeasurementVectorDimension, size_t ControlVectorDimension>
// class KF_Simple : LIB_XCORE_NAMESPACE::kalman_filter_t<StateVectorDimension, MeasurementVectorDimension, ControlVectorDimension> {
// public:
//   KF_Simple()
// };

#endif  //LIB_XCORE_LINALG_KALMAN_HPP
