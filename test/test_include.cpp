#include "embedded_cpp"
#include "dispatcher"
#include <iostream>

container::queue_t<int, 10, container::array_t> queue;
container::array_t<int, 1024>                   arr1;
container::heap_array_t<int, 1024>              arr2;

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

int main(int argc, char **argv) {
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

  container::bitset_t<32> bits;

  bits[1] = true;
  bits[3] = true;
  bits[3] &= false;

  for (size_t i = 0; i < bits.size(); ++i) {
    std::cout << bits[i];
  }

  return 0;
}
