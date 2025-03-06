#include <iostream>
#include <cassert>
#include "lib_xcore"

xcore::bitmap_allocator<int, 4> alloc;z

// Main
int main(int argc, char *argv[]) {
  auto ptr1 = alloc.acquire();
  auto ptr2 = alloc.acquire();
  auto ptr3 = alloc.acquire();
  auto ptr4 = alloc.acquire();
  auto ptr5 = alloc.acquire();

  std::cout << ptr1 << std::endl;
  std::cout << ptr2 << std::endl;
  std::cout << ptr3 << std::endl;
  std::cout << ptr4 << std::endl;
  std::cout << ptr5 << std::endl;

  alloc.release(ptr2);

  ptr5 = alloc.acquire();

  std::cout << ptr1 << std::endl;
  std::cout << ptr2 << std::endl;
  std::cout << ptr3 << std::endl;
  std::cout << ptr4 << std::endl;
  std::cout << ptr5 << std::endl;

  return 0;
}
