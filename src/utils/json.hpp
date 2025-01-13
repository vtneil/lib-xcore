#ifndef JSON_HPP
#define JSON_HPP

#include "internal/macros.hpp"

LIB_XCORE_BEGIN_NAMESPACE

template<typename BaseString>
struct json {
  // Helper class
  struct StringAppend {
    json         &parent;

    StringAppend &operator=(const char *value) {
      parent.str += BaseString(value).enquote('\"');
      if (++parent.counter > 0)
        parent.str += ",";
      return *this;
    }

    template<typename VT>
    StringAppend &operator=(const VT &value) {
      parent.str += BaseString(value);
      if (++parent.counter > 0)
        parent.str += ",";
      return *this;
    }

    template<typename VT>
    StringAppend &operator=(VT &&value) {
      parent.str += BaseString(forward<VT>(value));
      if (++parent.counter > 0)
        parent.str += ",";
      return *this;
    }
  };

  // Clear
  void clear() {
    counter      = 0;
    value_called = false;
    str.clear();
    str += "{";
  }

  // Assign value to key
  StringAppend operator[](const char *key) {
    str += '\"';
    str += key;
    str += "\":";
    return StringAppend{*this};
  }

  const BaseString &value() {
    if (value_called)
      return str;

    value_called = true;

    if (str.size() == 0)
      return str;

    if (str.size() == 1) {
      str += "}";
      return str;
    }

    str[str.size() - 1] = '}';
    return str;
  }

  // Implicit conversion to char array buffer
  [[nodiscard]] FORCE_INLINE constexpr operator const char *() noexcept {  // Implicit
    return value();
  }

private:
  BaseString str          = "{";
  size_t     counter      = 0;
  bool       value_called = false;
};

LIB_XCORE_END_NAMESPACE

#endif  //JSON_HPP
