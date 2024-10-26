#include "container/string.hpp"

namespace container::impl {
  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(const char *c_str)
      : base_string_t(c_str, strlen(c_str)) {}

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(const char *c_str, size_t n) {
    if (c_str)
      this->_copy(c_str, n);
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(const char c) {
    const char buf[] = {c, '\0'};
    *this            = ported::move(base_string_t(buf));
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(unsigned char value, unsigned char radix) {
    char buf[::utils::integral_buffer_size<unsigned char>()];
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(int value, unsigned char radix) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(unsigned int value, unsigned char radix) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(long value, unsigned char radix) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(unsigned long value, unsigned char radix) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(float value, unsigned int decimal_places) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(double value, unsigned int decimal_places) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(long long value, unsigned char radix) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::base_string_t(unsigned long long value, unsigned char radix) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container>::~base_string_t() {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container> &base_string_t<CharT, Capacity, Container>::operator=(const char *c_str) {
    *this = ported::move(base_string_t(c_str));
    return *this;
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::reserve(const size_t n) {
    if (this->_buffer() && capacity() > n)
      return true;

    this->_resize(n);

    if (!this->_buffer())
      return false;

    return true;
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  void base_string_t<CharT, Capacity, Container>::clear() {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(const base_string_t &) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(const char *) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(const char *, size_t) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(char) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(unsigned char) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(int) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(unsigned int) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(long) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(unsigned long) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(float) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(double) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(long long) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::concat(unsigned long long) {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::equals(const base_string_t &) const {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  bool base_string_t<CharT, Capacity, Container>::equals(const char *) const {
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  base_string_t<CharT, Capacity, Container> &base_string_t<CharT, Capacity, Container>::_copy(const char *c_str, const size_t n) {
    if (!this->reserve(n)) {
      this->_invalidate();
      return *this;
    }

    memmove(this->_buffer(), c_str, n + 1);
    this->_set_len(n);
    return *this;
  }

  template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
  void base_string_t<CharT, Capacity, Container>::_move(base_string_t &) {
  }
}  // namespace container::impl
