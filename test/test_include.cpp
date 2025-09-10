#include "lib_xcore"
#include "../src/xcore/dispatcher"
#include "math/numeric_vector.hpp"

#include <cassert>
#include <iostream>

xcore::container::queue_t<int, 10, xcore::container::array_t> queue;
xcore::container::array_t<int, 1024>                          arr1;
xcore::container::heap_array_t<int, 1024>                     arr2;

void vec_func(const xcore::numeric_vector<5> &v) {
  for (const auto &x: v) {
    std::cout << x << "\n";
  }
}

//
void test_sampler() {
  vec_func({1, 2});
  xcore::sampler_t<4, int> s;  // small buffer for easy testing

  // Initial state
  assert(s.count_under() == 0);
  assert(s.count_over() == 0);

  // Set threshold and add samples below/above
  s.set_threshold(10);

  s.add_sample(5);   // under
  s.add_sample(15);  // over
  s.add_sample(9);   // under
  s.add_sample(11);  // over
  std::cout << "Counts after 4 samples: under=" << s.count_under()
            << " over=" << s.count_over() << "\n";
  assert(s.count_under() == 2);
  assert(s.count_over() == 2);

  // Add one more (overwrites oldest: 5 under → evicted)
  s.add_sample(20);  // over
  std::cout << "Counts after overwrite: under=" << s.count_under()
            << " over=" << s.count_over() << "\n";
  assert(s.count_under() == 1);  // 15,9,11,20 = 9 under only
  assert(s.count_over() == 3);

  // Change threshold to 12, recount happens
  s.set_threshold(12);
  std::cout << "Counts after threshold=12: under=" << s.count_under()
            << " over=" << s.count_over() << "\n";
  // buffer contents are [15, 9, 11, 20] (circular, order doesn’t matter)
  // under = 9,11 → 2 ; over = 15,20 → 2
  assert(s.count_under() == 2);
  assert(s.count_over() == 2);

  // Ratios (check div-by-zero handling too)
  std::cout << "over/under=" << s.over_by_under()
            << " under/over=" << s.under_by_over() << "\n";

  // Reset
  s.reset();
  std::cout << "After reset: under=" << s.count_under()
            << " over=" << s.count_over() << "\n";
  assert(s.count_under() == 0);
  assert(s.count_over() == 0);

  std::cout << "All tests passed!\n";
}

uint32_t fake_time = 0;
uint32_t get_time() { return fake_time; }

void test_timer() {
  using namespace xcore;
  on_off_timer<uint32_t> t(1000, 2000, &get_time);

  for (int i = 0; i < 20; i++) {
    fake_time += 500;  // advance 0.5 second per loop
    std::cout << "[" << fake_time << " ms] ";

    t.on_rising([] {
       std::cout << "RISING edge → turned ON\n";
     })
      .on_falling();
  }
}

void print() {
  std::cout << "Hello!" << std::endl;
}

unsigned long millis() {
  static unsigned long t = 0;
  return t += 2;
}

void print_buf(const void *buf, const size_t n) {
  for (size_t i = 0; i < n; ++i) {
    std::cout << +*(static_cast<const char *>(buf) + i) << " ";
  }
}

void test_bitset() {
  xcore::bitset_t<64, int> b;

  for (size_t i = 0; i < b.capacity(); i += 2) {
    b[i] = true;
  }

  print_buf(static_cast<const uint8_t *>(b), b.capacity() / 8);
  for (size_t i = 1; i < b.capacity(); ++i) {
    std::cout << b[i];
  }
  std::cout << std::endl;
}

void test_nb() {
  xcore::nonblocking_delay<decltype(millis()), false> timeout(10, millis);
  timeout.reset();
  for (size_t i = 0; i < 20; ++i) {
    timeout([&]() -> void {
      std::cout << "RUN CALL" << std::endl;
    }).otherwise([&]() -> void {
      std::cout << "RUN OTHERWISE" << std::endl;
    });
  }

  auto task1 = []() -> void {
    std::cout << "RUN TASK #1" << std::endl;
  };

  auto task2 = []() -> void {
    std::cout << "RUN TASK #2" << std::endl;
  };

  xcore::Dispatcher<20> dispatcher;
  dispatcher << xcore::Task(task1, 1, millis);
  dispatcher << xcore::Task(task2, 10, millis);

  for (size_t i = 0; i < 20; ++i) {
    dispatcher();
  }
}

int main(int argc, char **argv) {
  test_sampler();
  test_timer();
  test_nb();
  test_bitset();

  queue.push(1);
  queue.push(2);
  queue.push(3);
  queue.emplace(4);
  queue.emplace(5);
  std::cout << queue.size() << "\n===\n";


  for (size_t i = 0; i < 10; ++i) {
    auto x = queue.pop();
    if (x) {
      std::cout << *x << "\n";
    } else {
      std::cout << "NA\n";
    }
  }

  std::cout << std::endl;

  xcore::container::bitset_t<32> bits;

  bits[1] = true;
  bits[3] = true;
  bits[3] &= false;

  for (size_t i = 0; i < bits.size(); ++i) {
    std::cout << bits[i];
  }

  return 0;
}
