#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <iostream>

// Your libs
#include <lib_xcore>
#include <xcore/math_module>

using namespace xcore;

constexpr real_t dt   = 0.1;
constexpr real_t hdts = 0.5 * dt * dt;

constexpr real_t base_noise_value = 0.1;  // used to init Q, R
constexpr int    N_STEPS          = 500;  // 50 seconds
constexpr real_t T_TOTAL          = N_STEPS * dt;

// Model matrices (given)
auto           F  = make_numeric_matrix<3, 3>({
  {1, dt, hdts},
  {0,  1,   dt},
  {0,  0,    1}
});
constexpr auto B  = make_numeric_matrix<3, 1>();  // zero control
constexpr auto H  = make_numeric_matrix<1, 3>({
  {1, 0, 0}
});
auto           Q0 = numeric_matrix<3, 3>::diagonals(base_noise_value);
auto           R0 = numeric_matrix<1, 1>::diagonals(base_noise_value);
constexpr auto x0 = make_numeric_vector<3>({0, 0, 0});  // {x, v, a}

// Large uncertainty on initial state
auto P0 = numeric_matrix<3, 3>::diagonals(real_t(1000));

/** Acceleration profile for the ground truth */
static inline real_t accel_profile(real_t t) {
  if (t < 10.0) return 0.0;
  else if (t < 25.0)
    return 0.5;
  else if (t < 35.0)
    return -0.7;
  else
    return 0.2 * std::sin(0.5 * t);
}

int main() {
  // RNG setup
  std::mt19937                     rng(42);
  std::normal_distribution<double> meas_noise(0.0, 0.8);    // measurement noise σ
  std::normal_distribution<double> spike_noise(0.0, 10.0);  // big outliers
  auto                             is_spike = [&](int k) {
    // Outlier every ~60 steps; tweak as desired
    return (k % 60) == 30 || (k % 60) == 0;
  };

  // Copy Q/R per filter (IAE/R-IAE will adapt their references)
  auto Q_base = Q0;
  auto R_base = R0;

  // --- Filters ---
  // Simple KF (fixed Q,R)
  kalman_filter_t<3, 1, 1> kf_simple(F, B, H, Q_base, R_base, x0, P0);

  // 4× IAE-KF
  auto Q_ia1 = Q0;
  auto R_ia1 = R0;
  auto Q_ia2 = Q0;
  auto R_ia2 = R0;
  auto Q_ia3 = Q0;
  auto R_ia3 = R0;
  auto Q_ia4 = Q0;
  auto R_ia4 = R0;

  iae_kalman_filter_t<3, 1, 1> kf_ia_005_0005(F, B, H, Q_ia1, R_ia1, x0, P0, 0.05, 0.005);
  iae_kalman_filter_t<3, 1, 1> kf_ia_005_0050(F, B, H, Q_ia2, R_ia2, x0, P0, 0.05, 0.05);
  iae_kalman_filter_t<3, 1, 1> kf_ia_020_0005(F, B, H, Q_ia3, R_ia3, x0, P0, 0.20, 0.005);
  iae_kalman_filter_t<3, 1, 1> kf_ia_020_0050(F, B, H, Q_ia4, R_ia4, x0, P0, 0.20, 0.05);

  // 3× R-IAE-KF (my chosen combos)
  auto Q_r1 = Q0;
  auto R_r1 = R0;
  auto Q_r2 = Q0;
  auto R_r2 = R0;
  auto Q_r3 = Q0;
  auto R_r3 = R0;

  // r-iae settings: (alpha, beta, tau, eps)
  r_iae_kalman_filter_t<3, 1, 1> kf_ria_1(F, B, H, Q_r1, R_r1, x0, P0, 0.20, 0.020, 3.0, 1e-12);
  r_iae_kalman_filter_t<3, 1, 1> kf_ria_2(F, B, H, Q_r2, R_r2, x0, P0, 0.20, 0.050, 2.5, 1e-12);
  r_iae_kalman_filter_t<3, 1, 1> kf_ria_3(F, B, H, Q_r3, R_r3, x0, P0, 0.10, 0.020, 3.5, 1e-12);

  // CSV
  std::ofstream ofs("kf_compare.csv");
  ofs << "time,actual,measurement,"
         "simple,"
         "iae_a05_b005,iae_a05_b05,iae_a2_b005,iae_a2_b05,"
         "riae_1,riae_2,riae_3\n";

  // Ground-truth state
  real_t x = 0, v = 0, a = 0;

  for (int k = 0; k < N_STEPS; ++k) {
    real_t t = k * dt;

    // --- Ground truth propagation with deterministic accel profile ---
    a = accel_profile(t);
    x = x + v * dt + real_t(0.5) * a * dt * dt;
    v = v + a * dt;

    // --- Measurement (position only) ---
    real_t z = x + real_t(meas_noise(rng));
    if (is_spike(k)) {
      z += real_t(spike_noise(rng));  // inject an outlier
    }

    // Wrap measurement into vector
    auto z_vec = make_numeric_vector<1>({z});

    // --- Run filters ---
    // Simple KF
    kf_simple.predict().update(z_vec);

    // IAE-KFs
    kf_ia_005_0005.predict().update(z_vec);
    kf_ia_005_0050.predict().update(z_vec);
    kf_ia_020_0005.predict().update(z_vec);
    kf_ia_020_0050.predict().update(z_vec);

    // R-IAE-KFs
    kf_ria_1.predict().update(z_vec);
    kf_ria_2.predict().update(z_vec);
    kf_ria_3.predict().update(z_vec);

    // --- Log row ---
    ofs << t << ','
        << x << ','
        << z << ','
        << kf_simple.state << ','
        << kf_ia_005_0005.state << ','
        << kf_ia_005_0050.state << ','
        << kf_ia_020_0005.state << ','
        << kf_ia_020_0050.state << ','
        << kf_ria_1.state << ','
        << kf_ria_2.state << ','
        << kf_ria_3.state << '\n';
  }

  ofs.close();
  std::cout << "Wrote kf_compare.csv (" << N_STEPS << " rows)\n";
  return 0;
}
