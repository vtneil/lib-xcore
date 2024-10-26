#ifndef STRING_HPP
#define STRING_HPP

#include "container/array.hpp"
#include "core/custom_numeric.hpp"
#include <cstring>

namespace container {
  namespace impl {
    template<typename CharT, size_t Capacity, template<typename, size_t> class Container = array_t>
    struct base_string_t {
    protected:
      using ArrayT = Container<CharT, Capacity>;

      ArrayT arr_  = {};
      size_t size_ = 0;

    public:
      // Constructors

      base_string_t(const char *c_str = "");
      base_string_t(const char *c_str, size_t n);
      base_string_t(const uint8_t *c_str, size_t n) : base_string_t(reinterpret_cast<const char *>(c_str), n) {}
      base_string_t(const base_string_t &)     = default;
      base_string_t(base_string_t &&) noexcept = default;
      base_string_t(char);
      base_string_t(unsigned char, unsigned char = 10);
      base_string_t(int, unsigned char = 10);
      base_string_t(unsigned int, unsigned char = 10);
      base_string_t(long, unsigned char = 10);
      base_string_t(unsigned long, unsigned char = 10);
      base_string_t(float, unsigned int decimal_places = 2);
      base_string_t(double, unsigned int decimal_places = 2);
      base_string_t(long long, unsigned char = 10);
      base_string_t(unsigned long long, unsigned char = 10);

      // Destructor
      ~base_string_t();

      // Assignment operators

      base_string_t &operator=(const base_string_t &)     = default;
      base_string_t &operator=(base_string_t &&) noexcept = default;
      base_string_t &operator=(const char *);

      // Size operations

      bool reserve(size_t);
      void clear();

      // String concatenations

      bool concat(const base_string_t &);
      bool concat(const char *);
      bool concat(const char *, size_t);
      bool concat(const uint8_t *c_str, const size_t n) { return concat(reinterpret_cast<const char *>(c_str), n); }
      bool concat(char);
      bool concat(unsigned char);
      bool concat(int);
      bool concat(unsigned int);
      bool concat(long);
      bool concat(unsigned long);
      bool concat(float);
      bool concat(double);
      bool concat(long long);
      bool concat(unsigned long long);

      template<typename VarT>
      base_string_t &operator+=(VarT &&v) {
        this->concat(ported::forward<VarT>(v));
        return *this;
      }

      // Comparison

      bool equals(const base_string_t &) const;
      bool equals(const char *) const;

      // Capacity

      [[nodiscard]] size_t size() const {
        return size_;
      }

      [[nodiscard]] size_t length() const {
        return size();
      }

      [[nodiscard]] size_t capacity() const {
        return arr_.size();
      }

      [[nodiscard]] bool is_empty() const {
        return size() == 0;
      }

    protected:
      base_string_t &_copy(const char *, size_t);
      void           _move(base_string_t &);

      // Resize
      void _resize(const size_t n) {
        arr_.dynamic_resize(n);
        if (size_ > capacity())
          size_ = capacity();
      }

      // Invalidate
      void _invalidate() {
        arr_.dynamic_clear();
        size_ = 0;
      }

      // Length
      void _set_len(const size_t n) {
        size_ = n;
        if (_buffer())
          _buffer()[n] = 0;
      }

      CharT *_buffer() {
        return static_cast<CharT *>(arr_);
      }

      const CharT *_buffer() const {
        return static_cast<const CharT *>(arr_);
      }
    };

  }  // namespace impl

  template<size_t Capacity>
  using string_t = impl::base_string_t<char, Capacity, array_t>;

  template<size_t Capacity>
  using heap_string_t    = impl::base_string_t<char, Capacity, heap_array_t>;

  using dynamic_string_t = impl::base_string_t<char, 0, dynamic_array_t>;
}  // namespace container

#endif  //STRING_HPP
