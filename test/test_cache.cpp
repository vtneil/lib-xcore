#include "lib_xcore"
#include <iostream>

uint32_t millis() {
  static uint32_t t = 0;
  return t++;
}

template<class cache_t>
void info(const cache_t &cache) {
  static size_t seq_no = 0;

  std::cout << "Sequence Number: " << seq_no++ << std::endl;
  std::cout << "Address: " << &cache << std::endl;
  std::cout << "Size: " << cache.size() << std::endl;
  std::cout << "Capacity: " << cache.capacity() << std::endl;

  std::cout << std::endl;
}

int main(int argc, char *argv[]) {
  xcore::container::lru_set_t<uint64_t, 4, millis> cache;

  info(cache);

  cache.insert(12345);
  cache.insert(23456);
  cache.insert(34567);
  cache.insert(45678);
  cache.insert(56789);
  cache.insert(67890);
  cache.insert(78901);
  info(cache);

  cache.remove(1);
  info(cache);

  cache.remove(78901);
  info(cache);

  auto v1 = cache.get(67890, true);
  if (v1) {
    const auto &time = xcore::get<1>(*v1);
    const auto &key  = xcore::get<2>(*v1);
    std::cout << time << " " << key << std::endl;
    xcore::get<2>(*v1) = 99999;
  }

  auto v2 = cache.get(99999);
  if (v2) {
    auto time = xcore::get<1>(*v2);
    auto key  = xcore::get<2>(*v2);
    std::cout << time << " " << key << std::endl << std::endl;
  }

  for (size_t i = 0; i < 32; ++i) {
    auto node = cache.rr_next(true);
    std::cout << xcore::get<0>(*node) << " "
              << xcore::get<1>(*node) << " "
              << xcore::get<2>(*node) << " "
              << std::endl;
  }
}
