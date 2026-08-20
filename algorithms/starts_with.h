#pragma once

#include "concepts.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <string_view>
#include <type_traits>

namespace mt {

/// True when `[first1, last1)` begins with `[first2, last2)`.
///
/// Example:
///   mt::starts_with("https://ex.com", "https://");  // true
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2>
[[nodiscard]] constexpr bool starts_with(I1 first1, S1 last1, I2 first2, S2 last2)
{
    while (first2 != last2) {
        if (first1 == last1 || !(*first1 == *first2)) {
            return false;
        }
        ++first1;
        ++first2;
    }
    return true;
}

template <std::ranges::input_range R1, std::ranges::input_range R2>
    requires(!std::convertible_to<std::remove_cvref_t<R1>, std::string_view> &&
             !std::convertible_to<std::remove_cvref_t<R1>, std::wstring_view>)
[[nodiscard]] constexpr bool starts_with(R1&& hay, R2&& needle)
{
    return starts_with(std::ranges::begin(hay), std::ranges::end(hay), std::ranges::begin(needle),
                       std::ranges::end(needle));
}

[[nodiscard]] inline bool starts_with(std::string_view hay, std::string_view needle)
{
    return hay.starts_with(needle);
}

[[nodiscard]] inline bool starts_with(std::wstring_view hay, std::wstring_view needle)
{
    return hay.starts_with(needle);
}

/// True when `[first1, last1)` ends with `[first2, last2)`. Bidirectional.
///
/// Example:
///   mt::ends_with("file.cpp", ".cpp");  // true
template <std::bidirectional_iterator I1, std::bidirectional_iterator I2>
[[nodiscard]] constexpr bool ends_with(I1 first1, I1 last1, I2 first2, I2 last2)
{
    while (first2 != last2) {
        if (first1 == last1) {
            return false;
        }
        --last1;
        --last2;
        if (!(*last1 == *last2)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool ends_with(std::string_view hay, std::string_view needle)
{
    return hay.ends_with(needle);
}

[[nodiscard]] inline bool ends_with(std::wstring_view hay, std::wstring_view needle)
{
    return hay.ends_with(needle);
}

[[nodiscard]] inline bool end_with(std::string_view hay, std::string_view needle)
{
    return ends_with(hay, needle);
}

[[nodiscard]] inline bool end_with(std::wstring_view hay, std::wstring_view needle)
{
    return ends_with(hay, needle);
}

}  // namespace mt
