#pragma once

#include <algorithm>
#include <iterator>

namespace mt {

/// Next combination of `[first, last)` that occupies `[first, middle)`.
/// Treats the range as two sides: selected `[first, middle)` and the rest.
/// Returns false when wrapping back to the first combination (then the range
/// is rotated into sorted order if it started sorted).
///
/// Howard Hinnant's algorithm. Precondition: `[first, middle)` and
/// `[middle, last)` are each sorted. After a true return, both sides are
/// sorted again.
///
/// Example:
///   std::vector<int> v{1, 2, 3, 4};
///   // first 2-combination: {1, 2} | {3, 4}
///   mt::next_combination(v.begin(), v.begin() + 2, v.end());
///   // {1, 3} | {2, 4}
template <std::bidirectional_iterator I>
bool next_combination(I first1, I last1, I first2, I last2)
{
    if (first1 == last1 || first2 == last2) {
        return false;
    }

    I m1 = last1;
    I m2 = last2;
    --m2;
    while (--m1 != first1 && !(*m1 < *m2)) {}
    bool result = (m1 == first1) && !(*first1 < *m2);
    if (!result) {
        while (first2 != m2 && !(*m1 < *first2)) {
            ++first2;
        }
        first1 = m1;
        std::iter_swap(first1, first2);
        ++first1;
        ++first2;
    }
    if (first1 != last1 && first2 != last2) {
        m1 = last1;
        m2 = first2;
        while (m1 != first1 && m2 != last2) {
            std::iter_swap(--m1, m2);
            ++m2;
        }
        std::reverse(first1, m1);
        std::reverse(first1, last1);
        std::reverse(m2, last2);
        std::reverse(first2, last2);
    }
    return !result;
}

template <std::bidirectional_iterator I>
bool next_combination(I first, I middle, I last)
{
    return next_combination(first, middle, middle, last);
}

}  // namespace mt
