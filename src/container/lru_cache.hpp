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
    using ObjectReference = ported::tuple<size_t, TimeT, KT &>;

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
        this->_insert_index(idx, ported::forward<KT>(key));
        this->_touch_index(idx);
      }
    }

    void insert(const KT& key) {
      insert(ported::forward<const KT>(key));
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

    ported::optional<ObjectReference> at(const size_t index, const bool touch = false) {
      if (index >= Capacity || !occupied_[index])
        return ported::nullopt;
      auto ret = ObjectReference(index, timestamps_[index], keys_[index]);
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

    ported::optional<ObjectReference> rr_next(const bool touch = true) {
      if (size_ == 0) return ported::nullopt;
      if (const auto idx_opt = this->_rr_hook(touch); idx_opt)
        return ObjectReference(*idx_opt, timestamps_[*idx_opt], keys_[*idx_opt]);
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

    ported::optional<size_t> _rr_hook(const bool touch) {
      if (size_ == 0) return ported::nullopt;

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

      return ported::nullopt;
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
      this->keys_[index]     = ported::move(key);
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
      return size_ < Capacity
               ? occupied_.find_first_false()  // Vacant
               : *_oldest_index();             // Full
    }
  };

  template<typename KT, typename VT, size_t Capacity, auto TimeFunc>
  class lru_map_t : public lru_set_t<KT, Capacity, TimeFunc> {
  public:
    using BitArray        = bitset_t<Capacity>;
    using TimeT           = decltype(TimeFunc());
    using ObjectReference = ported::tuple<size_t, TimeT, KT &, VT &>;

  protected:
    VT values_[Capacity] = {};  // Data

  public:
    lru_map_t() = default;

    // Methods

    void insert(KT &&key, VT &&value) {
      const size_t idx = this->_find(key).value_or(this->_find_free_entry());
      this->_insert_index(idx, ported::forward<KT>(key), ported::forward<VT>(value));
      this->_touch_index(idx);
    }

    void insert(const KT& key, const VT& value) {
      insert(ported::forward<const KT>(key), ported::forward<const VT>(value));
    }

    void insert(KT &&key) {
      static_assert(ported::is_constructible_v<VT>);
      this->insert(ported::forward<KT>(key), VT{});
    }

    void insert(const KT& key) {
      insert(ported::forward<const KT>(key));
    }

    template<typename... Args>
    void emplace(KT &&key, Args &&...args) {
      static_assert(ported::is_constructible_v<VT, Args &&...>);
      this->insert(ported::forward<KT>(key), VT(ported::forward<Args>(args)...));
    }

    ported::optional<ObjectReference> at(const size_t index, const bool touch = false) {
      if (index >= Capacity || !this->occupied_[index])
        return ported::nullopt;
      auto ret = ObjectReference(index, this->timestamps_[index], index, this->keys_[index], this->values_[index]);
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

    ported::optional<ObjectReference> rr_next(const bool touch = true) {
      if (this->size_ == 0) return ported::nullopt;
      if (const auto idx_opt = this->_rr_hook(touch); idx_opt)
        return ObjectReference(*idx_opt, this->timestamps_[*idx_opt], this->keys_[*idx_opt], this->values_[*idx_opt]);
      return ported::nullopt;
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
      this->keys_[index]     = ported::move(key);
      this->values_[index]   = ported::move(value);
    }
  };
}  // namespace container

#endif  //LRU_CACHE_HPP
