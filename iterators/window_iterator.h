#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Sliding windows of length `k` on a random-access range.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   for (auto w : mt::windows(v, 2)) {}  // {1,2}, {2,3}, {3,4}
template <std::random_access_iterator I>
class window_iterator {
public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::ranges::subrange<I>;
    using difference_type = std::iter_difference_t<I>;
    using reference = value_type;
    using pointer = void;

    window_iterator() = default;
    window_iterator(I current, I stop, difference_type k)
        : current_(current), stop_(stop), k_(k)
    {}

    [[nodiscard]] reference operator*() const { return {current_, current_ + k_}; }

    window_iterator& operator++()
    {
        if (current_ != stop_) {
            ++current_;
        }
        return *this;
    }

    window_iterator operator++(int)
    {
        window_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const window_iterator& a, const window_iterator& b)
    {
        return a.current_ == b.current_;
    }

private:
    I current_{};
    I stop_{};
    difference_type k_{1};
};

template <std::random_access_iterator I>
class window_range {
    I first_{};
    I last_{};
    std::iter_difference_t<I> k_{1};

public:
    window_range() = default;
    window_range(I first, I last, std::iter_difference_t<I> k)
        : first_(first), last_(last), k_(k)
    {}

    [[nodiscard]] window_iterator<I> begin() const
    {
        const I stop = end_pos();
        return {first_, stop, k_};
    }

    [[nodiscard]] window_iterator<I> end() const
    {
        const I stop = end_pos();
        return {stop, stop, k_};
    }

private:
    [[nodiscard]] I end_pos() const
    {
        if (k_ <= 0 || last_ - first_ < k_) {
            return first_;
        }
        return last_ - (k_ - 1);
    }
};

template <std::random_access_iterator I>
[[nodiscard]] window_range<I> windows(I first, I last, std::iter_difference_t<I> k)
{
    return {first, last, k};
}

template <std::ranges::random_access_range R>
[[nodiscard]] auto windows(R& range, std::ranges::range_difference_t<R> k)
{
    return windows(std::ranges::begin(range), std::ranges::end(range), k);
}

/// Non-overlapping chunks of length `k` (last chunk may be shorter).
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4, 5};
///   for (auto c : mt::chunks(v, 2)) {}  // {1,2}, {3,4}, {5}
template <std::random_access_iterator I>
class chunk_iterator {
public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::ranges::subrange<I>;
    using difference_type = std::iter_difference_t<I>;
    using reference = value_type;
    using pointer = void;

    chunk_iterator() = default;
    chunk_iterator(I current, I last, difference_type k) : current_(current), last_(last), k_(k) {}

    [[nodiscard]] reference operator*() const
    {
        const I e = current_ + std::min(k_, last_ - current_);
        return {current_, e};
    }

    chunk_iterator& operator++()
    {
        if (current_ != last_) {
            current_ += std::min(k_, last_ - current_);
        }
        return *this;
    }

    chunk_iterator operator++(int)
    {
        chunk_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const chunk_iterator& a, const chunk_iterator& b)
    {
        return a.current_ == b.current_;
    }

private:
    I current_{};
    I last_{};
    difference_type k_{1};
};

template <std::random_access_iterator I>
class chunk_range {
    I first_{};
    I last_{};
    std::iter_difference_t<I> k_{1};

public:
    chunk_range() = default;
    chunk_range(I first, I last, std::iter_difference_t<I> k)
        : first_(first), last_(last), k_(k)
    {}

    [[nodiscard]] chunk_iterator<I> begin() const
    {
        if (k_ <= 0 || first_ == last_) {
            return end();
        }
        return {first_, last_, k_};
    }
    [[nodiscard]] chunk_iterator<I> end() const { return {last_, last_, k_ <= 0 ? 1 : k_}; }
};

template <std::random_access_iterator I>
[[nodiscard]] chunk_range<I> chunks(I first, I last, std::iter_difference_t<I> k)
{
    return {first, last, k};
}

template <std::ranges::random_access_range R>
[[nodiscard]] auto chunks(R& range, std::ranges::range_difference_t<R> k)
{
    return chunks(std::ranges::begin(range), std::ranges::end(range), k);
}

}  // namespace mt
