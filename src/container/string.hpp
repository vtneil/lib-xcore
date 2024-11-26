#ifndef LIB_XCORE_CONTAINER_STRING_HPP
#define LIB_XCORE_CONTAINER_STRING_HPP

#include "core/string_format.hpp"
#include "container/array.hpp"
#include "core/custom_numeric.hpp"

#include <cstdarg>
#include <cstring>

namespace xcore::container {
  namespace impl {
    template<typename CharT, size_t Capacity, template<typename, size_t> class Container = array_t>
    struct lazy_add_string_t;
    // todo: Implement lazy addition to prevent heap fragmentation

    template<typename CharT, size_t Capacity, template<typename, size_t> class Container = array_t>
    struct basic_string_t {
    protected:
      using ArrayT = Container<CharT, Capacity>;

      ArrayT arr_  = {};
      size_t size_ = 0;

    public:
      // Constructors

      basic_string_t(const char *c_str = "")  // Implicit
          : basic_string_t(c_str, strlen(c_str)) {}

      basic_string_t(const char *c_str, const size_t n) {
        if (c_str)
          this->_copy(c_str, n);
      }

      basic_string_t(const uint8_t *c_str, size_t n)
          : basic_string_t(reinterpret_cast<const char *>(c_str), n) {}

      basic_string_t(const char c) {  // Implicit
        const char buf[] = {c, '\0'};
        *this            = move(basic_string_t(buf));
      }

      template<typename T, typename = enable_if_t<is_integral_v<T>>>
      basic_string_t(T value, const unsigned char radix = 10) {  // Implicit
        char buf[integral_buffer_size<T>()];
        xtostr<T>(value, buf, radix);
        *this = move(basic_string_t(buf));
      }

      template<typename T, typename = enable_if_t<is_floating_point_v<T>>>
      basic_string_t(T value, const unsigned int decimal_places = 2) {  // Implicit
        basic_string_t tmp_str{};
        size_t         n;

        if constexpr (is_same_v<remove_cv_t<T>, float>) {
          // Float
          n = decimal_places + 42;
        } else {
          // Double
          n = decimal_places + 312;
        }

        if (!tmp_str.reserve(n)) {
          tmp_str._invalidate();
          *this = move(basic_string_t("nan"));
          return;
        }

        xtostr<T>(value, tmp_str._buffer(), decimal_places + 2, decimal_places);
        *this = move(basic_string_t(tmp_str._buffer()));
      }

      template<size_t OCapacity, template<typename, size_t> class OContainer>
      explicit basic_string_t(const basic_string_t<CharT, OCapacity, OContainer> &other)
          : basic_string_t(other.c_str()) {}

      basic_string_t(const basic_string_t &other)     = default;
      basic_string_t(basic_string_t &&other) noexcept = default;

      // Destructor
      ~basic_string_t() {
        this->_invalidate();
      }

      // Assignment operators

      template<size_t OCapacity, template<typename, size_t> class OContainer>
      basic_string_t &operator=(const basic_string_t<CharT, OCapacity, OContainer> &other) {
        *this = other.c_str();
        return *this;
      }

      basic_string_t &operator=(const basic_string_t &other)     = default;
      basic_string_t &operator=(basic_string_t &&other) noexcept = default;

      basic_string_t &operator=(const char *c_str) {
        *this = move(basic_string_t(c_str));
        return *this;
      }

      // Size operations

      bool reserve(const size_t n) {
        if (this->_buffer() && this->capacity() > n)  // Includes terminating '\0'
          return true;

        this->_resize(n);

        if (!this->_buffer())
          return false;

        return true;
      }

      void shrink_to_fit() {
        this->_resize(strlen(this->_buffer()) + 1);
      }

      void clear() {
        this->_set_size(0);
      }

      // String concatenations

      template<size_t OCapacity, template<typename, size_t> class OContainer>
      bool concat(const basic_string_t<CharT, OCapacity, OContainer> &other) {
        return this->concat(other.c_str());
      }

      bool concat(const basic_string_t &other) {
        if (&other != this)
          return this->concat(other.c_str());

        const size_t new_size = 2 * this->size();

        if (!other.c_str()) return false;
        if (other.size() == 0) return true;
        if (!this->reserve(new_size)) {
          this->_invalidate();
          return false;
        }

        memmove(this->_buffer() + this->size(), this->_buffer(), this->size());
        this->_set_size(new_size);
        return true;
      }

      bool concat(const char *c_str, const size_t n) {
        const size_t new_size = size() + n;

        if (!c_str) return false;
        if (n == 0) return true;
        if (!this->reserve(new_size)) {
          this->_invalidate();
          return false;
        }

        memmove(this->_buffer() + this->size(), c_str, n + 1);
        this->_set_size(new_size);
        return true;
      }

      bool concat(const char *c_str) {
        return c_str ? concat(c_str, strlen(c_str)) : false;
      }

      bool concat(const uint8_t *c_str, const size_t n) {
        return concat(reinterpret_cast<const char *>(c_str), n);
      }

      bool concat(const char c) {
        const char buf[] = {c, '\0'};
        return concat(buf, 1);
      }

      // Integral overload
      template<typename T>
      enable_if_t<is_integral_v<T>, bool> concat(T value) {
        return this->concat(basic_string_t(value, 10));
      }

      // Floating point overload
      template<typename T>
      enable_if_t<is_floating_point_v<T>, bool> concat(T value) {
        return this->concat(basic_string_t(value, 2));
      }

      // Shortcut inplace addition
      template<typename VarT>
      basic_string_t &operator+=(VarT &&v) {
        this->concat(forward<VarT>(v));
        return *this;
      }

      // Addition
      template<typename VarT>
      basic_string_t operator+(VarT &&v) const {
        return this->copy() += v;
      }

      // Print
      int printf(const char *__restrict fmt, ...) __attribute__((format(printf, 2, 3))) {
        va_list ap;
        va_start(ap, fmt);
        const int len = vsnprintf(nullptr, 0, fmt, ap);
        va_end(ap);

        if (len < 0) {
          return 0;
        }

        reserve(len + 1);

        // Check for static string (reserve does nothing)
        if (capacity() < static_cast<size_t>(len + 1))
          return 0;

        va_start(ap, fmt);
        const int written = vsnprintf(_buffer(), len + 1, fmt, ap);
        va_end(ap);

        if (written < 0) {
          return 0;
        }

        _set_size(len);
        return len;
      }

      // Enquote
      basic_string_t enquote(const char quote) const {
        basic_string_t str{quote};
        str += *this;
        str += quote;
        return str;
      }

      // Comparison
      // todo: ==, !=, >, >=, <, <=

      // Capacity

      [[nodiscard]] size_t size() const { return this->size_; }
      [[nodiscard]] size_t length() const { return size(); }
      [[nodiscard]] size_t capacity() const { return this->arr_.size(); }
      [[nodiscard]] bool   is_empty() const { return size() == 0; }

      // Accessors/Iterators

      [[nodiscard]] FORCE_INLINE constexpr const CharT &operator[](const size_t index) const { return this->arr_[index]; }
      [[nodiscard]] FORCE_INLINE constexpr CharT       &operator[](const size_t index) { return this->arr_[index]; }

      [[nodiscard]] FORCE_INLINE constexpr const CharT *begin() const { return this->arr_.begin(); }
      [[nodiscard]] FORCE_INLINE constexpr CharT       *begin() { return this->arr_.begin(); }
      [[nodiscard]] FORCE_INLINE constexpr const CharT *cbegin() const { return this->arr_.cbegin(); }

      [[nodiscard]] FORCE_INLINE constexpr const CharT *end() const { return this->arr_.end(); }
      [[nodiscard]] FORCE_INLINE constexpr CharT       *end() { return this->arr_.end(); }
      [[nodiscard]] FORCE_INLINE constexpr const CharT *cend() const { return this->arr_.cend(); }

      [[nodiscard]] FORCE_INLINE constexpr const CharT *c_str() const { return this->_buffer(); }

      // Implicit conversion to char array buffer
      [[nodiscard]] FORCE_INLINE constexpr operator CharT *() noexcept {  // Implicit
        return this->_buffer();
      }

      // Implicit conversion to char array buffer
      [[nodiscard]] FORCE_INLINE constexpr operator const CharT *() const noexcept {  // Implicit
        return this->_buffer();
      }

      // Copy of this object
      basic_string_t copy() const {
        return basic_string_t(*this);
      }

    protected:
      basic_string_t &_copy(const char *c_str, const size_t n) {
        if (!this->reserve(n)) {
          this->_invalidate();
          return *this;
        }

        memmove(this->_buffer(), c_str, n + 1);
        this->_set_size(n);
        return *this;
      }

      // Resize Container
      void _resize(const size_t n) {
        this->arr_.dynamic_resize(n);
        if (this->size_ > capacity())
          this->size_ = capacity();
        this->_set_size(this->size_);
      }

      // Invalidate
      void _invalidate() {
        this->arr_.dynamic_clear();
        this->size_ = 0;
      }

      // Length
      void _set_size(const size_t n) {
        this->size_ = n;
        if (this->_buffer())
          this->_buffer()[n] = 0;
      }

      [[nodiscard]] FORCE_INLINE constexpr CharT *_buffer() {
        return this->begin();
      }

      [[nodiscard]] FORCE_INLINE constexpr const CharT *_buffer() const {
        return this->begin();
      }
    };

    template<typename CharT, size_t Capacity, template<typename, size_t> class Container>
    struct lazy_add_string_t {
    };
  }  // namespace impl

  template<size_t Capacity>
  using string_t = impl::basic_string_t<char, Capacity, array_t>;

  template<size_t Capacity>
  using heap_string_t    = impl::basic_string_t<char, Capacity, heap_array_t>;

  using dynamic_string_t = impl::basic_string_t<char, 0, dynamic_array_t>;
}  // namespace xcore::container

namespace xcore {
  using namespace container;
}

#endif  //LIB_XCORE_CONTAINER_STRING_HPP
