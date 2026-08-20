#pragma once

#include "concepts.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Random-access adaptor that walks an underlying iterator in strides of `step`.
/// Precondition: `step > 0`. Stores a logical index so comparisons never form a
/// pointer past one-past-the-end; `base()` clamps to the underlying `last`.
///
/// Example:
///   std::vector<int> v{0, 1, 2, 3, 4, 5};
///   auto s = mt::step(v, 2);
///   // yields 0, 2, 4
template <std::random_access_iterator I>
class step_iterator {
public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::iter_value_t<I>;
    using difference_type = std::iter_difference_t<I>;
    using reference = std::iter_reference_t<I>;
    using pointer = I;

    step_iterator() = default;
    step_iterator(I origin, I limit, difference_type index, difference_type step)
        : origin_(origin),
          limit_(limit),
          index_(index),
          step_(step)
    {}

    [[nodiscard]] reference operator*() const { return *(origin_ + index_ * step_); }
    [[nodiscard]] pointer operator->() const { return base(); }

    step_iterator& operator++()
    {
        ++index_;
        return *this;
    }

    step_iterator operator++(int)
    {
        step_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    step_iterator& operator--()
    {
        --index_;
        return *this;
    }

    step_iterator operator--(int)
    {
        step_iterator tmp = *this;
        --*this;
        return tmp;
    }

    step_iterator& operator+=(difference_type n)
    {
        index_ += n;
        return *this;
    }

    step_iterator& operator-=(difference_type n) { return *this += -n; }

    [[nodiscard]] reference operator[](difference_type n) const { return *(*this + n); }

    friend step_iterator operator+(step_iterator i, difference_type n) { return i += n; }
    friend step_iterator operator+(difference_type n, step_iterator i) { return i += n; }
    friend step_iterator operator-(step_iterator i, difference_type n) { return i += -n; }

    friend difference_type operator-(step_iterator a, step_iterator b)
    {
        return a.index_ - b.index_;
    }

    friend bool operator==(step_iterator a, step_iterator b)
    {
        return a.origin_ == b.origin_ && a.index_ == b.index_;
    }

    friend bool operator<(step_iterator a, step_iterator b)
    {
        return a.origin_ == b.origin_ ? a.index_ < b.index_ : a.origin_ < b.origin_;
    }
    friend bool operator>(step_iterator a, step_iterator b) { return b < a; }
    friend bool operator<=(step_iterator a, step_iterator b) { return !(b < a); }
    friend bool operator>=(step_iterator a, step_iterator b) { return !(a < b); }

    [[nodiscard]] I base() const
    {
        const difference_type span = limit_ - origin_;
        const difference_type offset = index_ * step_;
        if (offset >= span) {
            return limit_;
        }
        if (offset <= 0) {
            return origin_;
        }
        return origin_ + offset;
    }
    [[nodiscard]] difference_type step() const { return step_; }

private:
    I origin_{};
    I limit_{};
    difference_type index_{0};
    difference_type step_{1};
};

template <std::random_access_iterator I>
class step_range {
    I first_{};
    I last_{};
    std::iter_difference_t<I> step_{1};

public:
    step_range() = default;
    step_range(I first, I last, std::iter_difference_t<I> step)
        : first_(first),
          last_(last),
          step_(step)
    {}

    [[nodiscard]] step_iterator<I> begin() const { return {first_, last_, 0, step_}; }

    [[nodiscard]] step_iterator<I> end() const
    {
        using D = std::iter_difference_t<I>;
        const D n = last_ - first_;
        if (n <= 0 || step_ <= 0) {
            return {first_, last_, 0, step_};
        }
        const D count = (n + step_ - 1) / step_;
        return {first_, last_, count, step_};
    }
};

template <std::random_access_iterator I>
[[nodiscard]] step_range<I> step(I first, I last, std::iter_difference_t<I> stride)
{
    return {first, last, stride};
}

template <std::ranges::random_access_range R>
[[nodiscard]] auto step(R& range, std::ranges::range_difference_t<R> stride)
{
    return step(std::ranges::begin(range), std::ranges::end(range), stride);
}

}  // namespace mt
