#include "embedded_cpp"
#include "dispatcher"
#include <iostream>

container::circular_buffer_static_t<int, 32> queue;
Dispatcher<64>                               dispatcher;

//

void print() {
  std::cout << "Hello!" << std::endl;
}

unsigned long millis() {
  static unsigned long t = 0;
  return t++;
}

void loop();

USE_DISPATCHER(dispatcher);

int main(int argc, char **argv) {
  queue.put(1);
  queue.put(2);
  queue.put(3);
  queue.emplace(1);
  std::cout << queue.size() << "\n===\n";


  for (size_t i = 0; i < 10; ++i) {
    auto x = queue.get();
    if (x) {
      std::cout << *x << "\n";
    } else {
      std::cout << "NA\n";
    }
  }

  dispatcher << Task(print, 20, millis, 0);

  for (size_t i = 0; i < 100; ++i) {
    loop();
  }

  return 0;
}
