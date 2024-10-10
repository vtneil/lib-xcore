#include "embedded_cpp"
#include <iostream>

container::circular_buffer_static_t<int, 32> queue;

//

int main(int argc, char **argv) {
  queue.put(1);
  queue.put(2);
  queue.put(3);
  std::cout << queue.size() << "\n===\n";


  for (size_t i = 0; i < 10; ++i) {
    auto x = queue.get();
    if (x.has_value()) {
      std::cout << x.value() << "\n";
    } else {
      std::cout << "NA\n";
    }
  }
}
