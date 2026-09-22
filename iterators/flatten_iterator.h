#pragma once

#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace mt {

/// Flatten one level of nested forward ranges.
///
/// When `*outer` yields a temporary range, that range is owned for the duration
/// of the current inner iteration so `begin`/`end` are not taken from two
/// different temporaries.
///
/// Example:
///   std::vector<std::vector<int>> v{{1, 2}, {}, {3}};
///   for (int x : mt::flattened(v)) {}  // 1, 2, 3
template <std::forward_iterator Outer>
class flatten_iterator {
public:
    using InnerRange = std::remove_reference_t<std::iter_reference_t<Outer>>;
    using Inner = std::ranges::iterator_t<InnerRange>;
    using iterator_concept =
        std::conditional_t<std::is_lvalue_reference_v<std::iter_reference_t<Outer>>,
                           std::forward_iterator_tag, std::input_iterator_tag>;
    using iterator_category = iterator_concept;
    using value_type = std::iter_value_t<Inner>;
    using difference_type = std::iter_difference_t<Inner>;
    using reference = std::iter_reference_t<Inner>;
    using pointer = void;

    flatten_iterator() = default;
    flatten_iterator(Outer current, Outer last)
        : outer_(std::move(current)),
          outer_last_(std::move(last))
    {
        skip_empty();
    }

    [[nodiscard]] reference operator*() const { return *inner_; }

    flatten_iterator& operator++()
    {
        if (outer_ == outer_last_) {
            return *this;
        }
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
    void bind_inner()
    {
        if constexpr (std::is_lvalue_reference_v<std::iter_reference_t<Outer>>) {
            owned_.reset();
            inner_ = std::ranges::begin(*outer_);
            inner_last_ = std::ranges::end(*outer_);
        } else {
            owned_ = std::make_shared<InnerRange>(*outer_);
            inner_ = std::ranges::begin(*owned_);
            inner_last_ = std::ranges::end(*owned_);
        }
    }

    void skip_empty()
    {
        while (outer_ != outer_last_) {
            bind_inner();
            if (inner_ != inner_last_) {
                return;
            }
            ++outer_;
        }
        owned_.reset();
        inner_ = Inner{};
        inner_last_ = {};
    }

    Outer outer_{};
    Outer outer_last_{};
    Inner inner_{};
    std::ranges::sentinel_t<InnerRange> inner_last_{};
    std::shared_ptr<InnerRange> owned_{};
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
