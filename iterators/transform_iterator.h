#pragma once

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Lazy `map`: `*it` is `f(*base)`.
///
/// Example:
///   std::vector<int> v{1, 2, 3};
///   for (int x : mt::transform(v, [](int n) { return n * n; })) {}
template <std::input_iterator I, typename F>
class transform_iterator {
public:
    using iterator_concept = std::input_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = std::remove_cvref_t<std::invoke_result_t<F&, std::iter_reference_t<I>>>;
    using difference_type = std::iter_difference_t<I>;
    using reference = value_type;
    using pointer = void;

    transform_iterator() = default;
    transform_iterator(I base, F f) : base_(std::move(base)), f_(std::move(f)) {}

    [[nodiscard]] reference operator*() const { return std::invoke(f_, *base_); }

    transform_iterator& operator++()
    {
        ++base_;
        return *this;
    }

    transform_iterator operator++(int)
    {
        transform_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const transform_iterator& a, const transform_iterator& b)
    {
        return a.base_ == b.base_;
    }

    [[nodiscard]] I base() const { return base_; }

private:
    I base_{};
    [[no_unique_address]] F f_{};
};

template <std::input_iterator I, typename F>
class transform_range {
    I first_{};
    I last_{};
    F f_{};

public:
    transform_range() = default;
    transform_range(I first, I last, F f)
        : first_(std::move(first)), last_(std::move(last)), f_(std::move(f))
    {}

    [[nodiscard]] transform_iterator<I, F> begin() const { return {first_, f_}; }
    [[nodiscard]] transform_iterator<I, F> end() const { return {last_, f_}; }
};

template <std::input_iterator I, typename F>
[[nodiscard]] transform_range<I, F> transform(I first, I last, F f)
{
    return {std::move(first), std::move(last), std::move(f)};
}

template <std::ranges::input_range R, typename F>
[[nodiscard]] auto transform(R& range, F f)
{
    return transform(std::ranges::begin(range), std::ranges::end(range), std::move(f));
}

}  // namespace mt
