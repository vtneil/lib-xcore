#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <vector>

template<typename Engine>
void benchmark_engine(const std::string& engine_name, int num_iterations) {
  Engine engine;
  std::uniform_int_distribution<int> dist(0, 100);

  auto start = std::chrono::high_resolution_clock::now();

  int sum = 0;
  for (int i = 0; i < num_iterations; ++i) {
    sum += dist(engine);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;

  std::cout << std::setw(20) << engine_name << ": "
            << duration.count() << " seconds\n";
}

int main() {
  constexpr int num_iterations = 10'000'000;

  std::cout << "Benchmarking random engines with " << num_iterations
            << " iterations:\n\n";

  benchmark_engine<std::default_random_engine>("default_random_engine", num_iterations);
  benchmark_engine<std::minstd_rand>("minstd_rand", num_iterations);
  benchmark_engine<std::minstd_rand0>("minstd_rand0", num_iterations);
  benchmark_engine<std::mt19937>("mt19937", num_iterations);
  benchmark_engine<std::mt19937_64>("mt19937_64", num_iterations);
  benchmark_engine<std::ranlux24_base>("ranlux24_base", num_iterations);
  benchmark_engine<std::ranlux48_base>("ranlux48_base", num_iterations);

  return 0;
}
