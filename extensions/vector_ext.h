#pragma once

#include "concepts.h"

#include <algorithm>
#include <cstddef>
#include <istream>
#include <utility>
#include <vector>

namespace mt {

/// Sort then unique-erase. Adjacent duplicates after sorting disappear.
///
/// Example:
///   std::vector<int> v{3, 1, 3, 2, 1};
///   mt::distinct(v);  // {1, 2, 3}
template <typename T>
void distinct(std::vector<T>& v)
{
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

/// Keep elements that satisfy `pred` (stable). The rest are erased.
///
/// Example:
///   std::vector<int> v{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
///   mt::keep_if(v, [](int x) { return x < 8; });
///   // {0, 1, 4, 4, 0, 7, 7, 0}
template <typename T, typename P>
    requires std::predicate<P&, const T&>
void keep_if(std::vector<T>& v, P pred)
{
    v.erase(std::remove_if(v.begin(), v.end(),
                           [&](const T& x) { return !std::invoke(pred, x); }),
            v.end());
}

template <typename T, typename P>
    requires std::predicate<P&, const T&>
void keep_if_not(std::vector<T>& v, P pred)
{
    keep_if(v, [&](const T& x) { return !std::invoke(pred, x); });
}

/// Erase elements that satisfy `pred` (stable).
template <typename T, typename P>
    requires std::predicate<P&, const T&>
void delete_if(std::vector<T>& v, P pred)
{
    v.erase(std::remove_if(v.begin(), v.end(), std::move(pred)), v.end());
}

template <typename T, typename P>
    requires std::predicate<P&, const T&>
void delete_if_not(std::vector<T>& v, P pred)
{
    delete_if(v, [&](const T& x) { return !std::invoke(pred, x); });
}

/// O(1) erase at index `i` by swapping with the last element. Order is not
/// preserved. Precondition: `i < v.size()`.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   mt::fast_erase(v, 1);  // {1, 4, 3}
template <typename T>
void fast_erase(std::vector<T>& v, std::size_t i)
{
    v[i] = std::move(v.back());
    v.pop_back();
}

/// Append every `operator>>`-readable `T` from `in` onto `v`.
///
/// Example:
///   std::istringstream in{"1 2 3"};
///   std::vector<int> v;
///   mt::read(in, v);  // {1, 2, 3}
template <typename T>
void read(std::istream& in, std::vector<T>& v)
{
    T value{};
    while (in >> value) {
        v.push_back(std::move(value));
    }
}

inline void read_nums(std::istream& in, std::vector<int>& v)
{
    read(in, v);
}

}  // namespace mt
