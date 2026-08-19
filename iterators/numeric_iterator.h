#pragma once

#include "concepts.h"

#include <compare>
#include <iterator>
#include <utility>

namespace mt {

/// Random-access iterator whose value is an integer (counting / iota iterator).
///
/// Example:
///   std::vector<int> v;
///   std::copy(mt::numeric_iterator<int>{0}, mt::numeric_iterator<int>{4},
///             std::back_inserter(v));
///   // v == {0, 1, 2, 3}
template <Integer T>
class numeric_iterator {
public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = T;
    using pointer = void;

    numeric_iterator() = default;
    explicit numeric_iterator(T value) : value_(value) {}

    [[nodiscard]] T operator*() const { return value_; }

    numeric_iterator& operator++()
    {
        ++value_;
        return *this;
    }

    numeric_iterator operator++(int)
    {
        numeric_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    numeric_iterator& operator--()
    {
        --value_;
        return *this;
    }

    numeric_iterator operator--(int)
    {
        numeric_iterator tmp = *this;
        --*this;
        return tmp;
    }

    numeric_iterator& operator+=(difference_type n)
    {
        value_ += static_cast<T>(n);
        return *this;
    }

    numeric_iterator& operator-=(difference_type n) { return *this += -n; }

    [[nodiscard]] T operator[](difference_type n) const { return *(*this + n); }

    friend numeric_iterator operator+(numeric_iterator i, difference_type n)
    {
        return i += n;
    }

    friend numeric_iterator operator+(difference_type n, numeric_iterator i)
    {
        return i += n;
    }

    friend numeric_iterator operator-(numeric_iterator i, difference_type n)
    {
        return i += -n;
    }

    friend difference_type operator-(numeric_iterator a, numeric_iterator b)
    {
        return static_cast<difference_type>(a.value_) - static_cast<difference_type>(b.value_);
    }

    friend bool operator==(numeric_iterator, numeric_iterator) = default;
    friend std::strong_ordering operator<=>(numeric_iterator, numeric_iterator) = default;

private:
    T value_{};
};

/// Half-open integer range `[first, last)` as a pair of numeric iterators.
///
/// Example:
///   for (int x : mt::numeric_range(10, 13)) { ... }  // 10, 11, 12
template <Integer T>
class numeric_range {
    T first_{};
    T last_{};

public:
    using difference_type = std::ptrdiff_t;

    numeric_range() = default;
    numeric_range(T first, T last) : first_(first), last_(last) {}

    [[nodiscard]] numeric_iterator<T> begin() const { return numeric_iterator<T>{first_}; }
    [[nodiscard]] numeric_iterator<T> end() const { return numeric_iterator<T>{last_}; }
    [[nodiscard]] difference_type size() const
    {
        return static_cast<difference_type>(last_) - static_cast<difference_type>(first_);
    }
};

}  // namespace mt
