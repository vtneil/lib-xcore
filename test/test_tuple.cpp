#include "lib_xcore"
#include <iostream>
#include <cassert>

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

  std::cout << "All tests passed successfully.\n";
}

int main(int argc, char *argv[]) {
  auto tup = xcore::make_tuple(1, 2, 3);

  std::cout << xcore::get<0>(tup) << std::endl;
  std::cout << xcore::get<1>(tup) << std::endl;
  std::cout << xcore::get<2>(tup) << std::endl;

  const int x[0] = {};

  std::cout << x[0] << std::endl;

  auto p        = xcore::make_pair(1, 3);
  auto [p1, p2] = p;

  std::cout << p1 << " " << p2 << std::endl;

  test_bitset();

  return 0;
}
