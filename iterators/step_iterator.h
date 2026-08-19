#pragma once

#include "concepts.h"

#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Random-access adaptor that walks an underlying iterator in strides of `step`.
/// Precondition: `step > 0`.
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
    step_iterator(I current, difference_type step) : current_(current), step_(step) {}

    [[nodiscard]] reference operator*() const { return *current_; }
    [[nodiscard]] pointer operator->() const { return current_; }

    step_iterator& operator++()
    {
        current_ += step_;
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
        current_ -= step_;
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
        current_ += n * step_;
        return *this;
    }

    step_iterator& operator-=(difference_type n) { return *this += -n; }

    [[nodiscard]] reference operator[](difference_type n) const { return *(*this + n); }

    friend step_iterator operator+(step_iterator i, difference_type n) { return i += n; }
    friend step_iterator operator+(difference_type n, step_iterator i) { return i += n; }
    friend step_iterator operator-(step_iterator i, difference_type n) { return i += -n; }

    friend difference_type operator-(step_iterator a, step_iterator b)
    {
        return (a.current_ - b.current_) / a.step_;
    }

    friend bool operator==(step_iterator a, step_iterator b)
    {
        return a.current_ == b.current_;
    }

    friend bool operator<(step_iterator a, step_iterator b) { return a.current_ < b.current_; }
    friend bool operator>(step_iterator a, step_iterator b) { return b < a; }
    friend bool operator<=(step_iterator a, step_iterator b) { return !(b < a); }
    friend bool operator>=(step_iterator a, step_iterator b) { return !(a < b); }

    [[nodiscard]] I base() const { return current_; }
    [[nodiscard]] difference_type step() const { return step_; }

private:
    I current_{};
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
        : first_(first), last_(last), step_(step)
    {}

    [[nodiscard]] step_iterator<I> begin() const { return {first_, step_}; }

    [[nodiscard]] step_iterator<I> end() const
    {
        const auto n = last_ - first_;
        if (n <= 0 || step_ <= 0) {
            return {first_, step_};
        }
        const auto count = (n + step_ - 1) / step_;
        return {first_ + count * step_, step_};
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
