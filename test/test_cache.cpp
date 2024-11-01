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
    const auto &time = v1->timestamp;
    const auto &key  = v1->key;
    std::cout << time << " " << key << std::endl;
    v1->key = 99999;
  }

  auto v2 = cache.get(99999);
  if (v2) {
    auto time = v2->timestamp;
    auto key  = v2->key;
    std::cout << time << " " << key << std::endl
              << std::endl;
  }

  for (size_t i = 0; i < 32; ++i) {
    auto node = cache.rr_next(true);
    std::cout << node->index << " "
              << node->timestamp << " "
              << node->key << " "
              << std::endl;
  }
}
