#include "embedded_cpp"
#include "math_module"
#include <iostream>

void print_string(const char *str) {
  std::cout << "Address: " << memory::addressof(str[0]) << std::endl;
  std::cout << "Content: " << str << "\n"
            << std::endl;
}

int main(int argc, char *argv[]) {
  {
    container::string_t<512>      s1 = "This is stack string.";
    container::heap_string_t<512> s2 = "This is heap string.";
    container::dynamic_string_t   s3 = "This is dynamic string.";

    print_string(s1.c_str());
    print_string(s2.c_str());
    print_string(s3.c_str());
  }

  {
    container::string_t<512>      s1 = -123456;
    container::heap_string_t<512> s2 = -123456;
    container::dynamic_string_t   s3 = -123456;

    print_string(s1.c_str());
    print_string(s2.c_str());
    print_string(s3.c_str());
  }

  {
    container::string_t<512>      s1 = 1.88889f;
    container::heap_string_t<512> s2 = 1.88889f;
    container::dynamic_string_t   s3 = 1.88889f;

    print_string(s1.c_str());
    print_string(s2.c_str());
    print_string(s3.c_str());
  }

  {
    container::string_t<512>      s1 = 1.88889;
    container::heap_string_t<512> s2 = 1.88889;
    container::dynamic_string_t   s3 = 1.88889;

    print_string(s1.c_str());
    print_string(s2.c_str());
    print_string(s3.c_str());
  }

  {
    container::string_t<512>      s1 = "First string. ";
    container::heap_string_t<512> s2 = "Second string. ";
    container::dynamic_string_t   s3 = "Third string. ";

    s3.reserve(10240);

    s1 += s2;
    s1 += s1;
    s1 += s3;
    s3 += s1;

    print_string(s1);
    print_string(s3.c_str());
    std::cout << s1.capacity() << std::endl;
    std::cout << s3.capacity() << std::endl;

    container::string_t<512> s4(s3);
    print_string(s4.c_str());

    s4 = s2;
    print_string(s4.c_str());
    print_string((s4 + s1 + "X").c_str());
  }

  return 0;
}
