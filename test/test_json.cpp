#include "lib_xcore"
#include <iostream>

void print_string(const char *str) {
  std::cout << "Address: " << xcore::addressof(str[0]) << std::endl;
  std::cout << "Content: " << str << "\n"
            << std::endl;
}

int main(int argc, char *argv[]) {
  xcore::json<xcore::string_t<256>> payload;

  payload["key1"] = "value1";
  payload["key2"] = "value2";
  payload["key3"] = 9;
  payload["key3"] = xcore::string_t<32>(9.8888888, 12);

  print_string(payload.value());
}
