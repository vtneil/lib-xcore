#include "lib_xcore"
#include "dispatcher"
#include <iostream>

xcore::container::queue_t<int, 10, xcore::container::array_t> queue;
xcore::container::array_t<int, 1024>                          arr1;
xcore::container::heap_array_t<int, 1024>                     arr2;

//

void print() {
  std::cout << "Hello!" << std::endl;
}

unsigned long millis() {
  static unsigned long t = 0;
  return t++;
}

void print_buf(const void *buf, const size_t n) {
  for (size_t i = 0; i < n; ++i) {
    std::cout << +*(static_cast<const char *>(buf) + i) << " ";
  }
}

void test_bitset() {
  xcore::bitset_t<64, unsigned char> b;

  for (size_t i = 0; i < b.capacity(); i += 2) {
    b[i] = true;
  }

  print_buf(b, b.capacity() / 8);
  for (size_t i = 1; i < b.capacity(); ++i) {
    std::cout << b[i];
  }
  std::cout << std::endl;
}

int main(int argc, char **argv) {
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
