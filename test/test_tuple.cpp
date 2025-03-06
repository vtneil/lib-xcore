#include "lib_xcore"
#include <iostream>
#include <cassert>
#include <chrono>

void time_memcpy() {
  const int N        = 100000000;  // 100 million iterations
  uint8_t   bytes[4] = {0x78, 0x56, 0x34, 0x12};
  uint32_t  value    = 0;

  // Test reinterpret_cast
  auto start1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; ++i) {
    value = *reinterpret_cast<uint32_t *>(bytes);
  }
  auto end1 = std::chrono::high_resolution_clock::now();

  // Test memcpy
  auto start2 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; ++i) {
    memcpy(&value, bytes, sizeof(value));
  }
  auto end2 = std::chrono::high_resolution_clock::now();

  std::cout << "reinterpret_cast time: "
            << std::chrono::duration<double>(end1 - start1).count() << " seconds\n";
  std::cout << "memcpy time: "
            << std::chrono::duration<double>(end2 - start2).count() << " seconds\n";
}

void test_bitset() {
  using bitset32 = xcore::container::bitset_t<32, uint32_t>;

  bitset32 bs;

  // Test setting and getting a range within the same word
  bs.set(0, 8, 0b10101010);
  assert(bs.get(0, 8) == 0b10101010);
  std::cout << "Test 1 Passed: Range 0-8\n";

  // Test setting and getting across multiple words
  bs.set(8, 16, 0b11110000);
  assert(bs.get(8, 16) == 0b11110000);
  std::cout << "Test 2 Passed: Range 8-16\n";

  // Test setting a range that spans partial words
  bs.set(4, 20, 0b1100110011001100);
  assert(bs.get(4, 20) == 0b1100110011001100);
  std::cout << "Test 3 Passed: Range 4-20\n";

  // Test clearing all bits
  bs.clear_all();
  assert(bs.get(0, 32) == 0);
  std::cout << "Test 4 Passed: Clear All\n";

  // Test setting all bits
  bs.set_all();
  assert(bs.get(0, 32) == 0xFFFFFFFF);
  std::cout << "Test 5 Passed: Set All\n";

  // Test find_first_true, find_first_false, all, none, and any
  bs.clear_all();  // Start with all bits cleared
  assert(bs.none());
  assert(!bs.any());
  assert(!bs.all());
  assert(bs.find_first_true() == bs.size());
  assert(bs.find_first_false() == 0);
  std::cout << "Test 6 Passed: none, any, all, find_first_true, find_first_false on empty bitset\n";

  bs.set(1, true);  // Set only the second bit
  assert(!bs.none());
  assert(bs.any());
  assert(!bs.all());
  assert(bs.find_first_true() == 1);
  assert(bs.find_first_false() == 0);
  std::cout << "Test 7 Passed: none, any, all, find_first_true, find_first_false with one bit set\n";

  bs.set_all();  // Set all bits
  assert(!bs.none());
  assert(bs.any());
  assert(bs.all());
  assert(bs.find_first_true() == 0);
  assert(bs.find_first_false() == bs.size());
  std::cout << "Test 8 Passed: none, any, all, find_first_true, find_first_false on full bitset\n";

  bs.clear_all();   // Clear all bits
  bs.set(3, true);  // Set a specific bit
  assert(bs.find_first_true() == 3);
  assert(bs.find_first_false() == 0);
  bs.set(0, true);  // Also set the first bit
  assert(bs.find_first_true() == 0);
  assert(bs.find_first_false() == 1);
  std::cout << "Test 9 Passed: find_first_true, find_first_false with specific bits\n";

  bs.clear_all();
  for (size_t i = 0; i < bs.size(); ++i) {
    bs.set(i, (i % 2 == 0));  // Even indices set to 1
  }
  assert(bs.find_first_true() == 0);
  assert(bs.find_first_false() == 1);
  std::cout << "Test 10 Passed: Alternating bits test\n";

  std::cout << "All bitset tests completed successfully.\n";
}

void test_tuple_cat() {
  using namespace xcore;

  constexpr tuple<int, double> t1(1, 2.5);
  constexpr tuple<char, float> t2('a', 3.14f);

  constexpr auto               combined = tuple_cat(t1, t2);

  static_assert(is_same_v<decltype(combined), const tuple<int, double, char, float>>);

  // Access elements
  constexpr int v1 = get<0>(combined);  // 1
  static_assert(v1 == 1);
  constexpr double v2 = get<1>(combined);  // 2.5
  static_assert(v2 == 2.5);
  constexpr char v3 = get<2>(combined);  // 'a'
  static_assert(v3 == 'a');
  constexpr float v4 = get<3>(combined);  // 3.14f
  static_assert(v4 == 3.14f);

  {
    int                         a = 3;
    float                       b = 1.f;
    tuple<int &, int>           ta(a, 3);
    tuple<float &, const int &> tb(b, a);

    auto                        tc = tuple_cat(ta, tb);

    static_assert(is_same_v<decltype(tc), tuple<int &, int, float &, const int &>>);

    std::cout << a << std::endl;
    get<0>(tc) = 999;
    std::cout << a << std::endl;
  }
}

void test_repeated_tuple() {
  using namespace xcore;
  using namespace xcore::detail;

  int x    = 1;

  using TT = const int &;
  repeated_tuple<TT, 3> rp(x, x, x);
  static_assert(is_same_v<decltype(rp), tuple<TT, TT, TT>>);
}

int main(int argc, char *argv[]) {
  test_tuple_cat();
  test_repeated_tuple();

  auto tup = xcore::make_tuple(1, 2, 3);

  std::cout << xcore::get<0>(tup) << std::endl;
  std::cout << xcore::get<1>(tup) << std::endl;
  std::cout << xcore::get<2>(tup) << std::endl;

  auto &[a, b, c] = tup;

  std::cout << a << std::endl;
  std::cout << b << std::endl;
  std::cout << c << std::endl;

  const int x[0] = {};

  std::cout << x[0] << std::endl;

  auto p        = xcore::make_pair(1, 3);
  auto [p1, p2] = p;

  std::cout << p1 << " " << p2 << std::endl;

  test_bitset();

  time_memcpy();

  return 0;
}
