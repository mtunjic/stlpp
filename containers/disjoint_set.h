#pragma once

#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mt {

/// Union-find with union by rank and path compression.
///
/// Example:
///   mt::disjoint_set ds(3);
///   ds.unite(0, 1);
///   ds.connected(0, 1);  // true
///   ds.connected(0, 2);  // false
class disjoint_set {
    std::vector<std::size_t> parent_;
    std::vector<std::size_t> rank_;
    std::vector<std::size_t> size_;
    std::size_t components_ = 0;

public:
    explicit disjoint_set(std::size_t n)
        : parent_(n), rank_(n, 0), size_(n, 1), components_(n)
    {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    [[nodiscard]] std::size_t size() const { return parent_.size(); }
    [[nodiscard]] std::size_t components() const { return components_; }

    std::size_t find(std::size_t x)
    {
        check(x);
        std::size_t r = x;
        while (parent_[r] != r) {
            r = parent_[r];
        }
        while (parent_[x] != x) {
            const std::size_t next = parent_[x];
            parent_[x] = r;
            x = next;
        }
        return r;
    }

    bool unite(std::size_t a, std::size_t b)
    {
        a = find(a);
        b = find(b);
        if (a == b) {
            return false;
        }
        if (rank_[a] < rank_[b]) {
            std::swap(a, b);
        }
        parent_[b] = a;
        size_[a] += size_[b];
        if (rank_[a] == rank_[b]) {
            ++rank_[a];
        }
        --components_;
        return true;
    }

    [[nodiscard]] bool connected(std::size_t a, std::size_t b) { return find(a) == find(b); }

    [[nodiscard]] std::size_t component_size(std::size_t x) { return size_[find(x)]; }

private:
    void check(std::size_t x) const
    {
        if (x >= parent_.size()) {
            throw std::out_of_range("disjoint_set index");
        }
    }
};

}  // namespace mt
