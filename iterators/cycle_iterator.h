#pragma once

#include <cstddef>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Repeat `[first, last)` exactly `times` times (Haskell `cycle` + `take`).
///
/// Example:
///   std::vector<int> v{1, 2};
///   for (int x : mt::cycle(v, 3)) {}  // 1, 2, 1, 2, 1, 2
template <std::forward_iterator I>
class cycle_iterator {
public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::iter_value_t<I>;
    using difference_type = std::iter_difference_t<I>;
    using reference = std::iter_reference_t<I>;
    using pointer = void;

    cycle_iterator() = default;
    cycle_iterator(I begin, I end, I current, std::size_t trip, std::size_t times)
        : begin_(begin),
          end_(end),
          current_(current),
          trip_(trip),
          times_(times)
    {}

    [[nodiscard]] reference operator*() const { return *current_; }

    cycle_iterator& operator++()
    {
        if (times_ == 0 || trip_ >= times_) {
            return *this;
        }
        ++current_;
        if (current_ == end_) {
            current_ = begin_;
            ++trip_;
        }
        return *this;
    }

    cycle_iterator operator++(int)
    {
        cycle_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const cycle_iterator& a, const cycle_iterator& b)
    {
        return a.trip_ == b.trip_ && a.current_ == b.current_;
    }

private:
    I begin_{};
    I end_{};
    I current_{};
    std::size_t trip_ = 0;
    std::size_t times_ = 0;
};

template <std::forward_iterator I>
class cycle_range {
    I first_{};
    I last_{};
    std::size_t times_ = 0;

public:
    cycle_range() = default;
    cycle_range(I first, I last, std::size_t times) : first_(first), last_(last), times_(times) {}

    [[nodiscard]] cycle_iterator<I> begin() const
    {
        if (times_ == 0 || first_ == last_) {
            return end();
        }
        return {first_, last_, first_, 0, times_};
    }

    [[nodiscard]] cycle_iterator<I> end() const { return {first_, last_, first_, times_, times_}; }
};

template <std::forward_iterator I>
[[nodiscard]] cycle_range<I> cycle(I first, I last, std::size_t times)
{
    return {first, last, times};
}

template <std::ranges::forward_range R>
[[nodiscard]] auto cycle(R& range, std::size_t times)
{
    return cycle(std::ranges::begin(range), std::ranges::end(range), times);
}

}  // namespace mt
