#ifndef LIB_XCORE_CONTAINER_LRU_CACHE_HPP
#define LIB_XCORE_CONTAINER_LRU_CACHE_HPP

#include "core/ported_std.hpp"
#include "core/ported_optional.hpp"
#include "container/bitset.hpp"

namespace xcore::container {
  template<typename KT, size_t Capacity, auto TimeFunc>
  class lru_set_t {
    static_assert(Capacity > 0);

  public:
    using BitArray = bitset_t<Capacity>;
    using TimeT    = decltype(TimeFunc());

    struct entry_t {
      size_t index;
      TimeT  timestamp;
      KT    &key;
    };

  protected:
    BitArray occupied_             = {};  // Lookup
    TimeT    timestamps_[Capacity] = {};  // Lookup
    KT       keys_[Capacity]       = {};  // Lookup/data
    size_t   size_                 = {};  // Number of entries
    size_t   rr_index              = {};  // Round-robin index
    size_t   rr_ttl                = {};  // Round-robin time-to-live

  public:
    lru_set_t() = default;

    // Methods

    void insert(KT &&key) {
      if (const auto idx_opt = this->_find(key); idx_opt) {
        this->_touch_index(*idx_opt);
      } else {
        const size_t idx = this->_find_free_entry();
        this->_insert_index(idx, forward<KT>(key));
        this->_touch_index(idx);
      }
    }

    void insert(const KT &key) {
      if (const auto idx_opt = this->_find(key); idx_opt) {
        this->_touch_index(*idx_opt);
      } else {
        const size_t idx = this->_find_free_entry();
        this->_insert_index(idx, key);
        this->_touch_index(idx);
      }
    }

    void remove(const KT &key) {
      if (auto idx_opt = this->_find(key); idx_opt)
        this->_remove_index(*idx_opt);
    }

    void remove_by_index(const size_t index) {
      if (index < Capacity)
        this->_remove_index(index);
    }

    void remove_expired(const TimeT &expiry_age) {
      if (size_ == 0) return;

      const TimeT time = TimeFunc();
      for (size_t i = 0; i < Capacity; ++i) {
        if (this->occupied_[i] && time - this->timestamps_[i] > expiry_age)
          this->_remove_index(i);
      }
    }

    void touch(const KT &key) {
      if (const auto idx_opt = this->_find(key); idx_opt)
        this->_touch_index(*idx_opt);
    }

    optional<entry_t> at(const size_t index, const bool touch = false) {
      if (index >= Capacity || !occupied_[index])
        return nullopt;
      auto ret = entry_t{index, timestamps_[index], keys_[index]};
      if (touch)
        this->_touch_index(index);
      return ret;
    }

    optional<entry_t> get(const KT &key, const bool touch = false) {
      if (const auto idx_opt = this->_find(key); idx_opt)
        return at(*idx_opt, touch);
      return nullopt;
    }

    optional<entry_t> newest(const bool touch = false) {
      if (const auto idx_opt = this->_newest_index(); idx_opt)
        return at(*idx_opt, touch);
      return nullopt;
    }

    optional<entry_t> oldest(const bool touch = false) {
      if (const auto idx_opt = this->_oldest_index(); idx_opt)
        return at(*idx_opt, touch);
      return nullopt;
    }

    optional<entry_t> rr_next(const bool touch = true) {
      if (size_ == 0) return nullopt;
      if (const auto idx_opt = this->_rr_hook(touch); idx_opt)
        return entry_t{*idx_opt, timestamps_[*idx_opt], keys_[*idx_opt]};
      return nullopt;
    }

    void clear() {
      this->occupied_.clear_all();
    }

    bool contains(const KT &key) {
      return this->_find(key);
    }

    [[nodiscard]] constexpr size_t size() const {
      return this->size_;
    }

    [[nodiscard]] constexpr size_t capacity() const {
      return Capacity;
    }

  protected:
    optional<size_t> _find(const KT &key) const {
      if (size_ == 0) return nullopt;

      for (size_t i = 0; i < Capacity; ++i) {
        if (this->keys_[i] == key && this->occupied_[i])
          return i;
      }
      return nullopt;
    }

    optional<size_t> _rr_hook(const bool touch) {
      if (size_ == 0) return nullopt;

      if (rr_ttl == 0) {
        if (const auto oldest_idx = _oldest_index(); oldest_idx) {
          rr_index = *oldest_idx;
          rr_ttl   = size_;  // Reset TTL to size
          if (touch)
            this->_touch_index(rr_index);
          return rr_index;
        }
      }

      for (size_t i = 0; i < Capacity; ++i) {
        rr_index = (rr_index + 1) % Capacity;  // Move to next index
        if (occupied_[rr_index]) {
          --rr_ttl;  // Decrement TTL
          if (touch)
            this->_touch_index(rr_index);
          return rr_index;
        }
      }

      return nullopt;
    }

    void _insert_index(const size_t index, const KT &key) {
      if (!this->occupied_[index])
        ++this->size_;
      this->occupied_[index] = true;
      this->keys_[index]     = key;
    }

    void _insert_index(const size_t index, KT &&key) {
      if (!this->occupied_[index])
        ++this->size_;
      this->occupied_[index] = true;
      this->keys_[index]     = move(key);
    }

    void _remove_index(const size_t index) {
      if (this->occupied_[index])
        --this->size_;
      this->occupied_[index] = false;
    }

    void _touch_index(const size_t index) {
      this->timestamps_[index] = TimeFunc();
    }

    const KT &_key_at_index(const size_t index) const {
      return this->keys_[index];
    }

    [[nodiscard]] optional<size_t> _newest_index() const {
      if (size_ == 0) return nullopt;

      optional<size_t> newest_idx = nullopt;
      TimeT            max_t      = {};

      for (size_t i = 0; i < Capacity; ++i) {
        if (this->occupied_[i]) {
          if (!newest_idx || this->timestamps_[i] > max_t) {
            max_t      = this->timestamps_[i];
            newest_idx = i;
          }
        }
      }

      return newest_idx;
    }

    [[nodiscard]] optional<size_t> _oldest_index() const {
      if (size_ == 0) return nullopt;

      optional<size_t> oldest_idx = nullopt;
      TimeT            min_t      = {};

      for (size_t i = 0; i < Capacity; ++i) {
        if (this->occupied_[i]) {
          if (!oldest_idx || this->timestamps_[i] < min_t) {
            min_t      = this->timestamps_[i];
            oldest_idx = i;
          }
        }
      }

      return oldest_idx;
    }

    size_t _find_free_entry() {
      return size_ < Capacity
               ? occupied_.find_first_false()  // Vacant
               : *_oldest_index();             // Full
    }
  };

  template<typename KT, typename VT, size_t Capacity, auto TimeFunc>
  class lru_map_t : public lru_set_t<KT, Capacity, TimeFunc> {
  public:
    using BitArray = bitset_t<Capacity>;
    using TimeT    = decltype(TimeFunc());

    struct entry_t {
      size_t index;
      TimeT  timestamp;
      KT    &key;
      VT    &value;
    };

  protected:
    VT values_[Capacity] = {};  // Data

  public:
    lru_map_t() = default;

    // Methods

    void insert(KT &&key, VT &&value) {
      const size_t idx = this->_find(key).value_or(this->_find_free_entry());
      this->_insert_index(idx, forward<KT>(key), forward<VT>(value));
      this->_touch_index(idx);
    }

    void insert(const KT &key, const VT &value) {
      const size_t idx = this->_find(key).value_or(this->_find_free_entry());
      this->_insert_index(idx, key, value);
      this->_touch_index(idx);
    }

    void insert(KT &&key) {
      static_assert(is_constructible_v<VT>);
      this->insert(forward<KT>(key), VT{});
    }

    void insert(const KT &key) {
      static_assert(is_constructible_v<VT>);
      this->insert(forward<KT>(key), VT{});
    }

    template<typename... Args>
    void emplace(KT &&key, Args &&...args) {
      static_assert(is_constructible_v<VT, Args &&...>);
      this->insert(forward<KT>(key), VT(forward<Args>(args)...));
    }

    optional<entry_t> at(const size_t index, const bool touch = false) {
      if (index >= Capacity || !this->occupied_[index])
        return nullopt;
      auto ret = entry_t{index, this->timestamps_[index], this->keys_[index], this->values_[index]};
      if (touch)
        this->_touch_index(index);
      return ret;
    }

    optional<entry_t> get(const KT &key, const bool touch = false) {
      if (const auto idx_opt = this->_find(key); idx_opt)
        return at(*idx_opt, touch);
      return nullopt;
    }

    optional<entry_t> newest(const bool touch = false) {
      if (const auto idx_opt = this->_newest_index(); idx_opt)
        return at(*idx_opt, touch);
      return nullopt;
    }

    optional<entry_t> oldest(const bool touch = false) {
      if (const auto idx_opt = this->_oldest_index(); idx_opt)
        return at(*idx_opt, touch);
      return nullopt;
    }

    optional<entry_t> rr_next(const bool touch = true) {
      if (this->size_ == 0) return nullopt;
      if (const auto idx_opt = this->_rr_hook(touch); idx_opt)
        return entry_t{*idx_opt, this->timestamps_[*idx_opt], this->keys_[*idx_opt], this->values_[*idx_opt]};
      return nullopt;
    }

  protected:
    void _insert_index(const size_t index, const KT &key, const VT &value) {
      if (!this->occupied_[index])
        ++this->size_;
      this->occupied_[index] = true;
      this->keys_[index]     = key;
      this->values_[index]   = value;
    }

    void _insert_index(const size_t index, KT &&key, VT &&value) {
      if (!this->occupied_[index])
        ++this->size_;
      this->occupied_[index] = true;
      this->keys_[index]     = move(key);
      this->values_[index]   = move(value);
    }
  };
}  // namespace xcore::container

namespace xcore {
  using namespace container;
}

#endif  //LIB_XCORE_CONTAINER_LRU_CACHE_HPP
