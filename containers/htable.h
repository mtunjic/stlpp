#pragma once

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace mt {

/// Open-addressing hash table (linear probing). `K` must be hashable and
/// equality-comparable.
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
        K key{};
        V value{};
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
        buckets_.assign(n, slot{});
        size_ = 0;
        for (auto& s : old) {
            if (s.st == kind::filled) {
                insert(std::move(s.key), std::move(s.value));
            }
        }
    }

    slot* locate(const K& key)
    {
        std::size_t i = probe_start(key);
        for (std::size_t n = 0; n < buckets_.size(); ++n) {
            auto& s = buckets_[i];
            if (s.st == kind::empty) {
                return nullptr;
            }
            if (s.st == kind::filled && s.key == key) {
                return &s;
            }
            i = (i + 1) % buckets_.size();
        }
        return nullptr;
    }

    const slot* locate(const K& key) const
    {
        return const_cast<htable*>(this)->locate(key);
    }

public:
    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }

    void insert(K key, V value)
    {
        if (size_ * 4 >= buckets_.size() * 3) {
            rehash(buckets_.size() * 2);
        }
        for (;;) {
            std::size_t i = probe_start(key);
            std::size_t first_tomb = buckets_.size();
            std::size_t empty_i = buckets_.size();
            for (std::size_t n = 0; n < buckets_.size(); ++n) {
                auto& s = buckets_[i];
                if (s.st == kind::filled && s.key == key) {
                    s.value = std::move(value);
                    return;
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
            const std::size_t put = first_tomb != buckets_.size() ? first_tomb : empty_i;
            if (put == buckets_.size()) {
                rehash(buckets_.size() * 2);
                continue;
            }
            buckets_[put].st = kind::filled;
            buckets_[put].key = std::move(key);
            buckets_[put].value = std::move(value);
            ++size_;
            return;
        }
    }

    [[nodiscard]] V* find(const K& key)
    {
        slot* s = locate(key);
        return s ? &s->value : nullptr;
    }

    [[nodiscard]] const V* find(const K& key) const
    {
        const slot* s = locate(key);
        return s ? &s->value : nullptr;
    }

    bool erase(const K& key)
    {
        slot* s = locate(key);
        if (!s) {
            return false;
        }
        s->st = kind::tomb;
        --size_;
        return true;
    }
};

}  // namespace mt
