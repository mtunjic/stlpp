#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <vector>

namespace mt {

/// Merge two sorted ranges into `dest`.
///
/// Example:
///   mt::merge(std::vector{1, 3}, std::vector{2, 4});  // {1, 2, 3, 4}
template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Comp = std::less<>>
O merge(I1 a, S1 a_last, I2 b, S2 b_last, O dest, Comp comp = {})
{
    return std::merge(a, a_last, b, b_last, dest, std::move(comp));
}

template <std::ranges::input_range R1, std::ranges::input_range R2, typename Comp = std::less<>>
[[nodiscard]] auto merge(R1&& a, R2&& b, Comp comp = {})
{
    std::vector<std::ranges::range_value_t<R1>> out;
    merge(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b), std::ranges::end(b),
          std::back_inserter(out), std::move(comp));
    return out;
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Comp = std::less<>>
O set_union(I1 a, S1 a_last, I2 b, S2 b_last, O dest, Comp comp = {})
{
    return std::set_union(a, a_last, b, b_last, dest, std::move(comp));
}

template <std::ranges::input_range R1, std::ranges::input_range R2, typename Comp = std::less<>>
[[nodiscard]] auto set_union(R1&& a, R2&& b, Comp comp = {})
{
    std::vector<std::ranges::range_value_t<R1>> out;
    set_union(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
              std::ranges::end(b), std::back_inserter(out), std::move(comp));
    return out;
}

template <std::input_iterator I1, std::sentinel_for<I1> S1, std::input_iterator I2,
          std::sentinel_for<I2> S2, std::weakly_incrementable O, typename Comp = std::less<>>
O set_intersection(I1 a, S1 a_last, I2 b, S2 b_last, O dest, Comp comp = {})
{
    return std::set_intersection(a, a_last, b, b_last, dest, std::move(comp));
}

template <std::ranges::input_range R1, std::ranges::input_range R2, typename Comp = std::less<>>
[[nodiscard]] auto set_intersection(R1&& a, R2&& b, Comp comp = {})
{
    std::vector<std::ranges::range_value_t<R1>> out;
    set_intersection(std::ranges::begin(a), std::ranges::end(a), std::ranges::begin(b),
                     std::ranges::end(b), std::back_inserter(out), std::move(comp));
    return out;
}

}  // namespace mt
