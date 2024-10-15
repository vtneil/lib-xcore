#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include "core/ported_std.hpp"
#include "core/optional.hpp"
#include "container/bitset.hpp"

namespace container {
  template<typename KT, size_t Capacity, auto TimeFunc>
  class lru_set_t {
    static_assert(Capacity > 0);

  public:
    using BitArray = bitset_t<Capacity>;
    using TimeT    = decltype(TimeFunc());

  protected:
    BitArray occupied_             = {};  // Lookup
    TimeT    timestamps_[Capacity] = {};  // Lookup
    KT       keys_[Capacity]       = {};  // Lookup/data
    size_t   size_                 = {};

  public:
    lru_set_t() = default;

    // Methods

    void insert(const KT &key, const TimeT timestamp) {
      size_t idx = this->find(key);

      if (idx != Capacity) {
        // Table already has key, update the timestamp
        this->timestamps_[idx] = timestamp;
        return;
      }

      idx                    = this->_find_free_entry();
      this->keys_[idx]       = key;
      this->timestamps_[idx] = timestamp;
    }

    void remove_index(const size_t index) {
      this->occupied_.set(index, false);
      --this->size_;
    }

    void remove(const KT &key) {
      const size_t i = this->find(key);
      if (i == Capacity)
        return;
      this->remove_index(i);
    }

    void touch_index(const size_t index, const TimeT timestamp) {
      this->timestamps_[index] = timestamp;
    }

    void touch(const KT &key, const TimeT timestamp) {
      const size_t i = this->find(key);
      if (i == Capacity)
        return;
      this->touch_index(i, timestamp);
    }

    size_t find(const KT &key) const {
      for (size_t i = 0; i < Capacity; ++i) {
        if (!(this->keys_[i] == key))
          continue;
        if (this->occupied_[i])
          return i;
        return Capacity;
      }
      return Capacity;
    }

    const KT &key_at_index(const size_t index) const {
      return this->keys_[index];
    }

    const KT &key_at_index(const size_t index, const TimeT timestamp) {
      this->touch_index(index, timestamp);
      return this->key_at_index(index);
    }

    [[nodiscard]] size_t newest_index() const {
      if (size_ == 0)
        return Capacity;

      size_t idx   = 0;
      TimeT  max_t = this->timestamps_[0];
      for (size_t i = 1; i < Capacity; ++i) {
        if (this->occupied_.get(i) && this->timestamps_[i] > max_t) {
          max_t = this->timestamps_[i];
          idx   = i;
        }
      }

      if (!this->occupied_.get(0) && idx == 0)
        return Capacity;

      return idx;
    }

    [[nodiscard]] size_t newest_index(const TimeT timestamp) const {
      size_t idx = this->newest_index();
      touch_index(idx, timestamp);
      return idx;
    }

    const KT &newest_key() const {
      return this->key_at_index(newest_index());
    }

    const KT &newest_key(const TimeT timestamp) const {
      return this->key_at_index(newest_index(timestamp));
    }

    [[nodiscard]] size_t oldest_index() const {
      if (size_ == 0)
        return Capacity;

      size_t idx   = 0;
      TimeT  min_t = this->timestamps_[0];
      for (size_t i = 1; i < Capacity; ++i) {
        if (this->occupied_.get(i) && this->timestamps_[i] < min_t) {
          min_t = this->timestamps_[i];
          idx   = i;
        }
      }

      return idx;
    }

    [[nodiscard]] size_t oldest_index(const TimeT timestamp) const {
      size_t idx = this->oldest_index();
      touch_index(idx, timestamp);
      return idx;
    }

    const KT &oldest_key() const {
      return this->key_at_index(oldest_index());
    }

    const KT &oldest_key(const TimeT timestamp) const {
      return this->key_at_index(oldest_index(timestamp));
    }

    void clear() {
      this->occupied_.clear_all();
    }

    bool contains(const KT &key) {
      return find(key) != Capacity;
    }

    [[nodiscard]] constexpr size_t size() const {
      return this->size_;
    }

    [[nodiscard]] static constexpr size_t capacity() {
      return Capacity;
    }

    const BitArray            &occupied() const { return this->occupied_; }

    [[nodiscard]] const TimeT *timestamps() const { return this->timestamps_; }

    const KT                  *keys() const { return this->keys_; }

  protected:
    size_t _find_free_entry() {
      size_t idx;
      if (this->size_ < Capacity) {
        // Table is free, keep inserting
        idx = this->occupied_.find_first_false();
        this->occupied_.set(idx, true);
        ++this->size_;
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
    using TimeT = decltype(TimeFunc());

  protected:
    VT values_[Capacity] = {};  // Data

  public:
    lru_map_t() = default;

    // Methods

    void insert(const KT &key, const VT &value, const TimeT timestamp) {
      size_t idx = this->find(key);

      if (idx != Capacity) {
        // Table already has key, replace the value
        this->values_[idx]     = value;
        this->timestamps_[idx] = timestamp;
        return;
      }

      idx                    = this->_find_free_entry();
      this->keys_[idx]       = key;
      this->values_[idx]     = value;
      this->timestamps_[idx] = timestamp;
    }

    void insert(const KT &key, const TimeT timestamp) {  // Override
      this->insert(key, VT{}, timestamp);
    }

    VT &get_index(const size_t index, const TimeT timestamp) {
      this->timestamps_[index] = timestamp;
      return this->values_[index];
    }

    [[nodiscard]] const VT &get_index(const size_t index, const TimeT timestamp) const {
      this->timestamps_[index] = timestamp;
      return this->values_[index];
    }

    VT &get(const KT &key, const TimeT timestamp) {
      return this->get_index(find(key), timestamp);
    }

    [[nodiscard]] const VT &get(const KT &key, const TimeT timestamp) const {
      return this->get_index(find(key), timestamp);
    }

    const VT *values() const { return this->values_; }
  };
}  // namespace container

#endif  //LRU_CACHE_HPP
