#include <iostream>
#include <lib_xcore>
#include <xcore/math_module>

// Kalman Filters
constexpr size_t FILTER_ORDER = 3;
constexpr double BASE_NOISE   = 0.1;

constexpr xcore::numeric_matrix<FILTER_ORDER, 1>            B  = xcore::make_numeric_matrix<FILTER_ORDER, 1>();
constexpr xcore::numeric_matrix<1, FILTER_ORDER>            H  = xcore::make_numeric_matrix<1, FILTER_ORDER>({
  {
   1,
   }
});
constexpr xcore::numeric_vector<FILTER_ORDER>               x0 = xcore::make_numeric_vector<FILTER_ORDER>();
constexpr xcore::numeric_matrix<FILTER_ORDER, FILTER_ORDER> P0 = xcore::numeric_matrix<FILTER_ORDER, FILTER_ORDER>::diagonals(1000.);

struct FilterT {
  xcore::numeric_matrix<FILTER_ORDER, FILTER_ORDER> F;
  xcore::numeric_matrix<FILTER_ORDER, FILTER_ORDER> Q = xcore::numeric_matrix<FILTER_ORDER, FILTER_ORDER>::diagonals(BASE_NOISE);
  xcore::numeric_matrix<1, 1>                       R = xcore::numeric_matrix<1, 1>::diagonals(BASE_NOISE);

  xcore::kalman_filter_t<FILTER_ORDER, 1, 1> kf{F, B, H, Q, R, x0, P0};
  // xcore::r_iae_kalman_filter_t<FILTER_ORDER, 1, 1> kf{F, B, H, Q, R, x0, P0, 0.20, 0.050, 2.5, 1.e-12};
};

xcore::vdt<FILTER_ORDER - 1> vdt{0.100};

FilterT filter;

double t = 0;
double y = 0;

double f(const double x) {
  return sin(x);
}

int main(int argc, char *argv[]) {
  filter.F = vdt.generate_F();

  for (size_t i = 0; i < 1000; ++i) {
    static xcore::string_t<512> s = "";

    t                = static_cast<double>(i) / 10.;
    y                = f(t);
    const auto state = filter.kf.predict().update(y).state_vector();

    s = "";
    s += xcore::string_t<32>(y, 6);

    s += ",";
    s += xcore::string_t<32>(state[0], 6);

    s += "\n";

    std::cout << s;
  }

  return 0;
}
