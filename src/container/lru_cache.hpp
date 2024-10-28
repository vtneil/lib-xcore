#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include "core/ported_std.hpp"
#include "core/ported_optional.hpp"
#include "core/ported_tuple.hpp"
#include "container/bitset.hpp"

namespace container {
  template<typename KT, size_t Capacity, auto TimeFunc>
  class lru_set_t {
    static_assert(Capacity > 0);

  public:
    using BitArray        = bitset_t<Capacity>;
    using TimeT           = decltype(TimeFunc());
    using ObjectReference = ported::tuple<TimeT, KT &>;

  protected:
    BitArray occupied_             = {};  // Lookup
    TimeT    timestamps_[Capacity] = {};  // Lookup
    KT       keys_[Capacity]       = {};  // Lookup/data
    size_t   size_                 = {};

  public:
    lru_set_t() = default;

    // Methods

    void insert(const KT &key) {
      if (const auto idx_opt = this->_find(key); idx_opt) {
        this->_touch_index(*idx_opt);
      } else {
        const size_t idx = this->_find_free_entry();
        this->_insert_index(idx, key);
        this->_touch_index(idx);
      }
    }

    void insert(KT &&key) {
      if (const auto idx_opt = this->_find(key); idx_opt) {
        this->_touch_index(*idx_opt);
      } else {
        const size_t idx = this->_find_free_entry();
        this->_insert_index(idx, ported::forward<KT>(key));
        this->_touch_index(idx);
      }
    }

    void remove(const KT &key) {
      if (auto idx_opt = this->_find(key); idx_opt)
        this->_remove_index(*idx_opt);
    }

    void touch(const KT &key) {
      if (const auto idx_opt = this->_find(key); idx_opt)
        this->_touch_index(*idx_opt);
    }


    ported::optional<ObjectReference> at(const size_t index, const bool touch = false) {
      if (index >= Capacity || !occupied_[index])
        return ported::nullopt;
      auto ret = ObjectReference(timestamps_[index], keys_[index]);
      if (touch)
        this->_touch_index(index);
      return ret;
    }

    ported::optional<ObjectReference> get(const KT &key, const bool touch = false) {
      if (const auto idx_opt = this->_find(key); idx_opt)
        return at(*idx_opt, touch);
      return ported::nullopt;
    }

    ported::optional<ObjectReference> newest(const bool touch = false) {
      if (const auto idx_opt = this->_newest_index(); idx_opt)
        return at(*idx_opt, touch);
      return ported::nullopt;
    }

    ported::optional<ObjectReference> oldest(const bool touch = false) {
      if (const auto idx_opt = this->_oldest_index(); idx_opt)
        return at(*idx_opt, touch);
      return ported::nullopt;
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
    ported::optional<size_t> _find(const KT &key) const {
      if (size_ == 0) return ported::nullopt;

      for (size_t i = 0; i < Capacity; ++i) {
        if (this->keys_[i] == key && this->occupied_[i])
          return i;
      }
      return ported::nullopt;
    }

    void _insert_index(const size_t index, const KT &key) {
      this->occupied_[index] = true;
      this->keys_[index]     = key;
      ++this->size_;
    }

    void _insert_index(const size_t index, KT &&key) {
      this->occupied_[index] = true;
      this->keys_[index]     = ported::move(key);
      ++this->size_;
    }

    void _remove_index(const size_t index) {
      this->occupied_[index] = false;
      --this->size_;
    }

    void _touch_index(const size_t index) {
      this->timestamps_[index] = TimeFunc();
    }

    const KT &_key_at_index(const size_t index) const {
      return this->keys_[index];
    }

    [[nodiscard]] ported::optional<size_t> _newest_index() const {
      if (size_ == 0) return ported::nullopt;

      size_t idx   = 0;
      TimeT  max_t = this->timestamps_[0];

      for (size_t i = 1; i < Capacity; ++i) {
        if (this->occupied_[i] && this->timestamps_[i] > max_t) {
          max_t = this->timestamps_[i];
          idx   = i;
        }
      }

      if (!this->occupied_[0] && idx == 0) return ported::nullopt;
      return idx;
    }

    [[nodiscard]] ported::optional<size_t> _oldest_index() const {
      if (size_ == 0) return ported::nullopt;

      size_t idx   = 0;
      TimeT  min_t = this->timestamps_[0];

      for (size_t i = 1; i < Capacity; ++i) {
        if (this->occupied_[i] && this->timestamps_[i] < min_t) {
          min_t = this->timestamps_[i];
          idx   = i;
        }
      }

      return idx;
    }

    size_t _find_free_entry() {
      size_t idx;
      if (this->size_ < Capacity) {
        // Table is free, keep inserting
        idx = this->occupied_.find_first_false();
      } else {
        // Table is full, replace the oldest entry
        idx         = 0;
        TimeT min_t = this->timestamps_[0];
        for (size_t i = 1; i < Capacity; ++i) {
          if (this->timestamps_[i] < min_t) {
            min_t = this->timestamps_[i];
            idx   = i;
          }
        }
      }

      return idx;
    }
  };

  template<typename KT, typename VT, size_t Capacity, auto TimeFunc>
  class lru_map_t : public lru_set_t<KT, Capacity, TimeFunc> {
  public:
    using BitArray        = bitset_t<Capacity>;
    using TimeT           = decltype(TimeFunc());
    using ObjectReference = ported::tuple<TimeT, KT &, VT &>;

  protected:
    VT values_[Capacity] = {};  // Data

  public:
    lru_map_t() = default;

    // Methods

    void insert(const KT &key, const VT &value) {
      if (const auto idx_opt = this->_find(key); idx_opt) {
        this->_touch_index(*idx_opt);
      } else {
        const size_t idx = this->_find_free_entry();
        this->_insert_index(idx, key, value);
        this->_touch_index(idx);
      }
    }

    void insert(KT &&key, const VT &value) {
      if (const auto idx_opt = this->_find(key); idx_opt) {
        this->_touch_index(*idx_opt);
      } else {
        const size_t idx = this->_find_free_entry();
        this->_insert_index(idx, ported::forward<KT>(key), ported::forward<VT>(value));
        this->_touch_index(idx);
      }
    }

    void insert(const KT &key) {
      static_assert(ported::is_constructible_v<VT>);
      this->insert(key, VT{});
    }

    void insert(KT &&key) {
      static_assert(ported::is_constructible_v<VT>);
      this->insert(key, ported::forward<VT>(VT{}));
    }

    ported::optional<ObjectReference> at(const size_t index, const bool touch = false) {
      if (index >= Capacity || !this->occupied_[index])
        return ported::nullopt;
      auto ret = ObjectReference(this->timestamps_[index], this->keys_[index], this->values_[index]);
      if (touch)
        this->_touch_index(index);
      return ret;
    }

    ported::optional<ObjectReference> get(const KT &key, const bool touch = false) {
      if (const auto idx_opt = this->_find(key); idx_opt)
        return at(*idx_opt, touch);
      return ported::nullopt;
    }

    ported::optional<ObjectReference> newest(const bool touch = false) {
      if (const auto idx_opt = this->_newest_index(); idx_opt)
        return at(*idx_opt, touch);
      return ported::nullopt;
    }

    ported::optional<ObjectReference> oldest(const bool touch = false) {
      if (const auto idx_opt = this->_oldest_index(); idx_opt)
        return at(*idx_opt, touch);
      return ported::nullopt;
    }

  protected:
    void _insert_index(const size_t index, const KT &key, const VT &value) {
      this->occupied_[index] = true;
      this->keys_[index]     = key;
      this->values_[index]   = value;
      ++this->size_;
    }

    void _insert_index(const size_t index, KT &&key, VT &&value) {
      this->occupied_[index] = true;
      this->keys_[index]     = ported::move(key);
      this->values_[index]   = ported::move(value);
      ++this->size_;
    }
  };
}  // namespace container

#endif  //LRU_CACHE_HPP
