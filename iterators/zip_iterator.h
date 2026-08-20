#pragma once

#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace mt {

/// Lazy `zip`. Stops when either range ends.
///
/// Example:
///   std::vector a{1, 2}, b{10, 20, 30};
///   for (auto [x, y] : mt::zip_view(a, b)) {}  // (1,10), (2,20)
template <std::input_iterator A, std::input_iterator B>
class zip_iterator {
public:
    using iterator_concept = std::input_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = std::pair<std::iter_value_t<A>, std::iter_value_t<B>>;
    using difference_type =
        std::common_type_t<std::iter_difference_t<A>, std::iter_difference_t<B>>;
    using reference = value_type;
    using pointer = void;

    zip_iterator() = default;
    zip_iterator(A a, B b) : a_(std::move(a)), b_(std::move(b)) {}

    [[nodiscard]] reference operator*() const
    {
        return {static_cast<std::iter_value_t<A>>(*a_), static_cast<std::iter_value_t<B>>(*b_)};
    }

    zip_iterator& operator++()
    {
        ++a_;
        ++b_;
        return *this;
    }

    zip_iterator operator++(int)
    {
        zip_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const zip_iterator& x, const zip_iterator& y)
    {
        return x.a_ == y.a_ && x.b_ == y.b_;
    }

    [[nodiscard]] bool exhausted(const A& a_last, const B& b_last) const
    {
        return a_ == a_last || b_ == b_last;
    }

private:
    A a_{};
    B b_{};
};

/// Sentinel for `zip_iterator`: equal when either underlying range is exhausted.
template <std::input_iterator A, std::input_iterator B>
class zip_sentinel {
public:
    zip_sentinel() = default;
    zip_sentinel(A a_last, B b_last) : a_last_(std::move(a_last)), b_last_(std::move(b_last)) {}

    friend bool operator==(const zip_iterator<A, B>& it, const zip_sentinel& s)
    {
        return it.exhausted(s.a_last_, s.b_last_);
    }

private:
    A a_last_{};
    B b_last_{};
};

template <std::input_iterator A, std::input_iterator B>
class zip_view_range {
    A a_first_{};
    A a_last_{};
    B b_first_{};
    B b_last_{};

public:
    zip_view_range() = default;
    zip_view_range(A a_first, A a_last, B b_first, B b_last)
        : a_first_(std::move(a_first)),
          a_last_(std::move(a_last)),
          b_first_(std::move(b_first)),
          b_last_(std::move(b_last))
    {}

    [[nodiscard]] zip_iterator<A, B> begin() const { return {a_first_, b_first_}; }
    [[nodiscard]] zip_sentinel<A, B> end() const { return {a_last_, b_last_}; }
};

template <std::input_iterator A, std::input_iterator B>
[[nodiscard]] zip_view_range<A, B> zip_view(A a_first, A a_last, B b_first, B b_last)
{
    return {std::move(a_first), std::move(a_last), std::move(b_first), std::move(b_last)};
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
[[nodiscard]] auto zip_view(R1& a, R2& b)
{
    return zip_view(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
                    std::ranges::end(b));
}

}  // namespace mt
