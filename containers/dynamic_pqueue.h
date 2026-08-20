#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mt {

/// Binary heap with handles so values can be updated after insertion
/// (Dijkstra-style decrease-key). `std::less` is a max-heap; use `std::greater`
/// for a min-heap.
///
/// Example:
///   mt::dynamic_pqueue<int, std::greater<int>> q;  // min-heap
///   auto h = q.push(5);
///   q.update(h, 1);
///   q.top();  // 1
template <typename T, typename Compare = std::less<T>>
class dynamic_pqueue {
    struct node {
        T value{};
        std::size_t handle = 0;
    };

    std::vector<node> heap_;
    std::vector<std::size_t> where_;  // handle -> heap index, or npos
    std::vector<std::size_t> free_;
    Compare less_{};

    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    void swap_nodes(std::size_t i, std::size_t j)
    {
        std::swap(heap_[i], heap_[j]);
        where_[heap_[i].handle] = i;
        where_[heap_[j].handle] = j;
    }

    void sift_up(std::size_t i)
    {
        while (i > 0) {
            const std::size_t p = (i - 1) / 2;
            if (!less_(heap_[p].value, heap_[i].value)) {
                break;
            }
            swap_nodes(i, p);
            i = p;
        }
    }

    void sift_down(std::size_t i)
    {
        for (;;) {
            std::size_t best = i;
            const std::size_t l = 2 * i + 1;
            const std::size_t r = l + 1;
            if (l < heap_.size() && less_(heap_[best].value, heap_[l].value)) {
                best = l;
            }
            if (r < heap_.size() && less_(heap_[best].value, heap_[r].value)) {
                best = r;
            }
            if (best == i) {
                break;
            }
            swap_nodes(i, best);
            i = best;
        }
    }

public:
    using handle = std::size_t;

    [[nodiscard]] bool empty() const { return heap_.empty(); }
    [[nodiscard]] std::size_t size() const { return heap_.size(); }

    handle push(T value)
    {
        handle h;
        if (!free_.empty()) {
            h = free_.back();
            free_.pop_back();
            where_[h] = heap_.size();
        } else {
            h = where_.size();
            where_.push_back(heap_.size());
        }
        heap_.push_back(node{std::move(value), h});
        sift_up(heap_.size() - 1);
        return h;
    }

    [[nodiscard]] const T& top() const
    {
        if (heap_.empty()) {
            throw std::out_of_range("dynamic_pqueue::top");
        }
        return heap_.front().value;
    }
    [[nodiscard]] handle top_handle() const
    {
        if (heap_.empty()) {
            throw std::out_of_range("dynamic_pqueue::top_handle");
        }
        return heap_.front().handle;
    }

    void pop()
    {
        if (heap_.empty()) {
            return;
        }
        const handle h = heap_.front().handle;
        swap_nodes(0, heap_.size() - 1);
        heap_.pop_back();
        where_[h] = npos;
        free_.push_back(h);
        if (!heap_.empty()) {
            sift_down(0);
        }
    }

    [[nodiscard]] bool contains(handle h) const { return h < where_.size() && where_[h] != npos; }

    void update(handle h, T value)
    {
        if (!contains(h)) {
            return;
        }
        const std::size_t i = where_[h];
        heap_[i].value = std::move(value);
        sift_up(i);
        sift_down(i);
    }
};

}  // namespace mt
