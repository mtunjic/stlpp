#pragma once

#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

namespace mt {

/// Skip elements that fail `pred` (lazy `keep_if`).
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   for (int x : mt::filter(v, [](int n) { return n % 2 == 0; })) {}  // 2, 4
template <std::forward_iterator I, typename P>
class filter_iterator {
public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::iter_value_t<I>;
    using difference_type = std::iter_difference_t<I>;
    using reference = std::iter_reference_t<I>;
    using pointer = void;

    filter_iterator() = default;
    filter_iterator(I current, I last, P pred)
        : current_(std::move(current)), last_(std::move(last)), pred_(std::move(pred))
    {
        satisfy();
    }

    [[nodiscard]] reference operator*() const { return *current_; }

    filter_iterator& operator++()
    {
        if (current_ != last_) {
            ++current_;
            satisfy();
        }
        return *this;
    }

    filter_iterator operator++(int)
    {
        filter_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const filter_iterator& a, const filter_iterator& b)
    {
        return a.current_ == b.current_;
    }

private:
    void satisfy()
    {
        while (current_ != last_ && !std::invoke(pred_, *current_)) {
            ++current_;
        }
    }

    I current_{};
    I last_{};
    [[no_unique_address]] P pred_{};
};

template <std::forward_iterator I, typename P>
class filter_range {
    I first_{};
    I last_{};
    P pred_{};

public:
    filter_range() = default;
    filter_range(I first, I last, P pred)
        : first_(std::move(first)), last_(std::move(last)), pred_(std::move(pred))
    {}

    [[nodiscard]] filter_iterator<I, P> begin() const { return {first_, last_, pred_}; }
    [[nodiscard]] filter_iterator<I, P> end() const { return {last_, last_, pred_}; }
};

template <std::forward_iterator I, typename P>
[[nodiscard]] filter_range<I, P> filter(I first, I last, P pred)
{
    return {std::move(first), std::move(last), std::move(pred)};
}

template <std::ranges::forward_range R, typename P>
[[nodiscard]] auto filter(R& range, P pred)
{
    return filter(std::ranges::begin(range), std::ranges::end(range), std::move(pred));
}

}  // namespace mt
