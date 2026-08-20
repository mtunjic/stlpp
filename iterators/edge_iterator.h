#pragma once

#include "concepts.h"

#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Forward iterator over consecutive pairs `(a[i], a[i+1])` — the edges of a
/// path. A range of n vertices has n-1 edges. Empty and singleton ranges have
/// no edges.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   for (auto [from, to] : mt::edges(v)) {
///       // (1,2), (2,3), (3,4)
///   }
template <std::forward_iterator I>
class edge_iterator {
public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<std::iter_value_t<I>, std::iter_value_t<I>>;
    using difference_type = std::iter_difference_t<I>;
    using reference = value_type;
    using pointer = void;

    edge_iterator() = default;
    edge_iterator(I current, I limit) : current_(current), limit_(limit) {}

    [[nodiscard]] reference operator*() const
    {
        I next = current_;
        ++next;
        return {static_cast<std::iter_value_t<I>>(*current_),
                static_cast<std::iter_value_t<I>>(*next)};
    }

    edge_iterator& operator++()
    {
        ++current_;
        return *this;
    }

    edge_iterator operator++(int)
    {
        edge_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(edge_iterator a, edge_iterator b) { return a.current_ == b.current_; }

    [[nodiscard]] I base() const { return current_; }
    [[nodiscard]] I limit() const { return limit_; }

private:
    I current_{};
    I limit_{};
};

template <std::forward_iterator I>
class edge_range {
    I first_{};
    I last_{};

public:
    edge_range() = default;
    edge_range(I first, I last) : first_(first), last_(last) {}

    [[nodiscard]] edge_iterator<I> begin() const { return {first_, last_}; }

    [[nodiscard]] edge_iterator<I> end() const
    {
        I cur = first_;
        if (cur == last_) {
            return {cur, last_};
        }
        I nxt = cur;
        ++nxt;
        if (nxt == last_) {
            return {first_, last_};
        }
        if constexpr (std::bidirectional_iterator<I>) {
            return {std::prev(last_), last_};
        } else {
            I e = first_;
            I n = std::next(e);
            while (std::next(n) != last_) {
                ++e;
                ++n;
            }
            return {std::next(e), last_};
        }
    }
};

template <std::forward_iterator I>
[[nodiscard]] edge_range<I> edges(I first, I last)
{
    return {first, last};
}

template <std::ranges::forward_range R>
[[nodiscard]] auto edges(R& range)
{
    return edges(std::ranges::begin(range), std::ranges::end(range));
}

}  // namespace mt
