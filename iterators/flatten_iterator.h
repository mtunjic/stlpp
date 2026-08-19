#pragma once

#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Flatten one level of nested forward ranges.
///
/// Example:
///   std::vector<std::vector<int>> v{{1, 2}, {}, {3}};
///   for (int x : mt::flattened(v)) {}  // 1, 2, 3
template <std::forward_iterator Outer>
class flatten_iterator {
public:
    using Inner = std::ranges::iterator_t<std::iter_reference_t<Outer>>;
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::iter_value_t<Inner>;
    using difference_type = std::iter_difference_t<Inner>;
    using reference = std::iter_reference_t<Inner>;
    using pointer = void;

    flatten_iterator() = default;
    flatten_iterator(Outer current, Outer last)
        : outer_(std::move(current)), outer_last_(std::move(last))
    {
        skip_empty();
    }

    [[nodiscard]] reference operator*() const { return *inner_; }

    flatten_iterator& operator++()
    {
        ++inner_;
        if (inner_ == inner_last_) {
            ++outer_;
            skip_empty();
        }
        return *this;
    }

    flatten_iterator operator++(int)
    {
        flatten_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const flatten_iterator& a, const flatten_iterator& b)
    {
        return a.outer_ == b.outer_ && (a.outer_ == a.outer_last_ || a.inner_ == b.inner_);
    }

private:
    void skip_empty()
    {
        while (outer_ != outer_last_) {
            inner_ = std::ranges::begin(*outer_);
            inner_last_ = std::ranges::end(*outer_);
            if (inner_ != inner_last_) {
                return;
            }
            ++outer_;
        }
    }

    Outer outer_{};
    Outer outer_last_{};
    Inner inner_{};
    Inner inner_last_{};
};

template <std::forward_iterator Outer>
class flatten_range {
    Outer first_{};
    Outer last_{};

public:
    flatten_range() = default;
    flatten_range(Outer first, Outer last) : first_(std::move(first)), last_(std::move(last)) {}

    [[nodiscard]] flatten_iterator<Outer> begin() const { return {first_, last_}; }
    [[nodiscard]] flatten_iterator<Outer> end() const { return {last_, last_}; }
};

template <std::forward_iterator Outer>
[[nodiscard]] flatten_range<Outer> flattened(Outer first, Outer last)
{
    return {std::move(first), std::move(last)};
}

template <std::ranges::forward_range R>
[[nodiscard]] auto flattened(R& range)
{
    return flattened(std::ranges::begin(range), std::ranges::end(range));
}

}  // namespace mt
