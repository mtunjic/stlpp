#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace mt {

/// Open-addressing hash table (linear probing). `K` must be hashable and
/// equality-comparable. Empty and tombstone slots do not construct `K` or `V`.
///
/// `insert` fails if the key is already present. Use `insert_or_assign` to
/// overwrite.
///
/// Example:
///   mt::htable<int, std::string> t;
///   t.insert(3, "three");
///   auto* v = t.find(3);  // "three"
template <typename K, typename V, typename Hash = std::hash<K>>
class htable {
    enum class kind { empty, filled, tomb };

    struct slot {
        kind st = kind::empty;
        std::optional<K> key;
        std::optional<V> value;

        void emplace(K&& k, V&& v)
        {
            key.emplace(std::move(k));
            value.emplace(std::move(v));
            st = kind::filled;
        }

        void bury()
        {
            key.reset();
            value.reset();
            st = kind::tomb;
        }
    };

    std::vector<slot> buckets_{8};
    std::size_t size_ = 0;
    Hash hash_{};

    [[nodiscard]] std::size_t probe_start(const K& key) const
    {
        return hash_(key) % buckets_.size();
    }

    void rehash(std::size_t n)
    {
        std::vector<slot> old = std::move(buckets_);
        buckets_.assign(n == 0 ? 8 : n, slot{});
        size_ = 0;
        for (auto& s : old) {
            if (s.st == kind::filled) {
                insert_or_assign(std::move(*s.key), std::move(*s.value));
            }
        }
    }

    slot* locate(const K& key)
    {
        if (buckets_.empty()) {
            return nullptr;
        }
        std::size_t i = probe_start(key);
        for (std::size_t n = 0; n < buckets_.size(); ++n) {
            auto& s = buckets_[i];
            if (s.st == kind::empty) {
                return nullptr;
            }
            if (s.st == kind::filled && *s.key == key) {
                return &s;
            }
            i = (i + 1) % buckets_.size();
        }
        return nullptr;
    }

    const slot* locate(const K& key) const { return const_cast<htable*>(this)->locate(key); }

    template <typename Put>
    bool place(K key, V value, Put put)
    {
        if (buckets_.empty() || size_ * 4 >= buckets_.size() * 3) {
            rehash(buckets_.empty() ? 8 : buckets_.size() * 2);
        }
        for (;;) {
            std::size_t i = probe_start(key);
            std::size_t first_tomb = buckets_.size();
            std::size_t empty_i = buckets_.size();
            for (std::size_t n = 0; n < buckets_.size(); ++n) {
                auto& s = buckets_[i];
                if (s.st == kind::filled && *s.key == key) {
                    return put(s, std::move(key), std::move(value));
                }
                if (s.st == kind::tomb && first_tomb == buckets_.size()) {
                    first_tomb = i;
                }
                if (s.st == kind::empty) {
                    empty_i = i;
                    break;
                }
                i = (i + 1) % buckets_.size();
            }
            const std::size_t dest = first_tomb != buckets_.size() ? first_tomb : empty_i;
            if (dest == buckets_.size()) {
                rehash(buckets_.size() * 2);
                continue;
            }
            buckets_[dest].emplace(std::move(key), std::move(value));
            ++size_;
            return true;
        }
    }

public:
    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }

    /// Insert only if `key` is absent. Returns true when a new slot was filled.
    bool insert(K key, V value)
    {
        return place(std::move(key), std::move(value), [](slot&, K&&, V&&) { return false; });
    }

    /// Insert or overwrite. Returns true when a new key was inserted.
    bool insert_or_assign(K key, V value)
    {
        return place(std::move(key), std::move(value), [](slot& s, K&&, V&& v) {
            s.value = std::move(v);
            return false;
        });
    }

    [[nodiscard]] V* find(const K& key)
    {
        slot* s = locate(key);
        return s ? &*s->value : nullptr;
    }

    [[nodiscard]] const V* find(const K& key) const
    {
        const slot* s = locate(key);
        return s ? &*s->value : nullptr;
    }

    [[nodiscard]] bool contains(const K& key) const { return locate(key) != nullptr; }

    bool erase(const K& key)
    {
        slot* s = locate(key);
        if (!s) {
            return false;
        }
        s->bury();
        --size_;
        return true;
    }
};

}  // namespace mt
