#ifndef PORTED_OPTIONAL_HPP
#define PORTED_OPTIONAL_HPP

#include "memory.hpp"
#include "core/ported_std.hpp"

namespace ported {
  struct nullopt_t {
    explicit constexpr nullopt_t() = default;
  };

  constexpr nullopt_t nullopt{};

  template<typename T>
  struct optional {
  private:
    bool          is_initialized;
    unsigned char storage[memory::nearest_alignment<T, void *>()];

  public:
    optional() : is_initialized(false) {}

    // Implicit
    optional(nullopt_t) : is_initialized(false) {}

    // Implicit
    optional(const T &value) : is_initialized(true) {
      new (storage) T(value);
    }

    // Implicit
    optional(T &&value) : is_initialized(true) {
      new (storage) T(move(value));
    }

    optional(const optional &other) : is_initialized(other.is_initialized) {
      if (is_initialized) {
        new (storage) T(*other);
      }
    }

    optional(optional &&other) noexcept : is_initialized(other.is_initialized) {
      if (is_initialized) {
        new (storage) T(move(*other));
        other.reset();
      }
    }

    ~optional() {
      reset();
    }

    optional &operator=(const optional &other) {
      if (this != &other) {
        reset();
        if (other.is_initialized) {
          new (storage) T(*other);
          is_initialized = true;
        }
      }
      return *this;
    }

    optional &operator=(optional &&other) noexcept {
      if (this != &other) {
        reset();
        if (other.is_initialized) {
          new (storage) T(move(*other));
          is_initialized = true;
          other.reset();
        }
      }
      return *this;
    }

    optional &operator=(const T &value) {
      if (is_initialized) {
        **this = value;
      } else {
        new (storage) T(value);
        is_initialized = true;
      }
      return *this;
    }

    optional &operator=(T &&value) {
      if (is_initialized) {
        **this = move(value);
      } else {
        new (storage) T(move(value));
        is_initialized = true;
      }
      return *this;
    }

    optional &operator=(nullopt_t) {
      reset();
      return *this;
    }

    void reset() {
      if (is_initialized) {
        reinterpret_cast<T *>(storage)->~T();
        is_initialized = false;
      }
    }

    T *operator->() {
      return reinterpret_cast<T *>(storage);
    }

    const T *operator->() const {
      return reinterpret_cast<const T *>(storage);
    }

    T &operator*() {
      return *reinterpret_cast<T *>(storage);
    }

    const T &operator*() const {
      return *reinterpret_cast<const T *>(storage);
    }

    explicit operator bool() const {
      return is_initialized;
    }

    [[nodiscard]] bool has_value() const {
      return is_initialized;
    }

    T &value() {
      return **this;
    }

    const T &value() const {
      return **this;
    }

    template<typename U>
    constexpr T &value_or(U &&default_value) const {
      static_assert(ported::is_constructible_v<U &&, T>);

      if (this->has_value())
        return this->value();

      return static_cast<T>(ported::forward<U>(default_value));
    }

    template<typename U>
    constexpr T &&value_or(U &&default_value) {
      static_assert(ported::is_constructible_v<U &&, T>);

      if (this->has_value())
        return this->value();

      return static_cast<T>(ported::forward<U>(default_value));
    }
  };

  template<typename T, typename... Args>
  constexpr optional<T> make_optional(Args &&...args) {
    return optional<T>(ported::forward<Args>(args)...);
  }
}  // namespace ported

#endif  //PORTED_OPTIONAL_HPP
