#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#include "memory/allocator.hpp"
#include "lib/move.hpp"

namespace ported {
  template<typename T>
  class Optional {
  private:
    bool is_initialized;
    alignas(T) unsigned char storage[sizeof(T)];

  public:
    Optional() : is_initialized(false) {}

    Optional(const T &value) : is_initialized(true) {
      new (storage) T(value);  // In-place new
    }

    Optional(T &&value) : is_initialized(true) {
      new (storage) T(std::move(value));
    }

    Optional(const Optional &other) : is_initialized(other.is_initialized) {
      if (is_initialized) {
        new (storage) T(*other);
      }
    }

    Optional(Optional &&other) noexcept : is_initialized(other.is_initialized) {
      if (is_initialized) {
        new (storage) T(std::move(*other));
        other.reset();
      }
    }

    ~Optional() {
      reset();
    }

    Optional &operator=(const Optional &other) {
      if (this != &other) {
        reset();
        if (other.is_initialized) {
          new (storage) T(*other);
          is_initialized = true;
        }
      }
      return *this;
    }

    Optional &operator=(Optional &&other) noexcept {
      if (this != &other) {
        reset();
        if (other.is_initialized) {
          new (storage) T(std::move(*other));
          is_initialized = true;
          other.reset();
        }
      }
      return *this;
    }

    Optional &operator=(const T &value) {
      if (is_initialized) {
        **this = value;
      } else {
        new (storage) T(value);
        is_initialized = true;
      }
      return *this;
    }

    Optional &operator=(T &&value) {
      if (is_initialized) {
        **this = std::move(value);
      } else {
        new (storage) T(std::move(value));
        is_initialized = true;
      }
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

    bool has_value() const {
      return is_initialized;
    }

    T &value() {
      return **this;
    }

    const T &value() const {
      return **this;
    }
  };

}  // namespace ported

#endif  //OPTIONAL_HPP
