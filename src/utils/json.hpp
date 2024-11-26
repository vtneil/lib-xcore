#ifndef JSON_HPP
#define JSON_HPP

namespace xcore {
  template<typename BaseString>
  struct json {
    // Helper class
    struct StringAppend {
      json &parent;

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
        parent.str += BaseString(xcore::forward<VT>(value));
        if (++parent.counter > 0)
          parent.str += ",";
        return *this;
      }
    };

    // Assign value to key
    StringAppend operator[](const char *key) {
      str += '\"';
      str += key;
      str += "\":";
      return StringAppend{*this};
    }

    const BaseString &value() {
      str[str.size() - 1] = '}';
      return str;
    }

  private:
    BaseString str     = "{";
    size_t     counter = 0;
  };
}  // namespace xcore

#endif  //JSON_HPP
