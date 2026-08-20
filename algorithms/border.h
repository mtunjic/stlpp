#pragma once

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <vector>

namespace mt {

/// Surround a 1-D range with `fill` on both ends.
///
/// Example:
///   mt::border(std::vector<int>{1, 2, 3}, 0);  // {0, 1, 2, 3, 0}
template <std::ranges::input_range R>
[[nodiscard]] std::vector<std::ranges::range_value_t<R>>
border(R&& range, const std::ranges::range_value_t<R>& fill)
{
    using T = std::ranges::range_value_t<R>;
    std::vector<T> out;
    out.push_back(fill);
    for (auto&& x : range) {
        out.push_back(static_cast<T>(x));
    }
    out.push_back(fill);
    return out;
}

/// Surround a rectangular grid with a frame of `fill`.
///
/// Example:
///   std::vector<std::vector<int>> g{{1, 2}, {3, 4}};
///   auto b = mt::border(g, 0);
///   // {{0,0,0,0}, {0,1,2,0}, {0,3,4,0}, {0,0,0,0}}
template <typename T>
[[nodiscard]] std::vector<std::vector<T>> border(const std::vector<std::vector<T>>& grid,
                                                 const T& fill)
{
    if (grid.empty()) {
        return {{fill, fill}, {fill, fill}};
    }
    std::size_t cols = 0;
    for (const auto& row : grid) {
        cols = std::max(cols, row.size());
    }
    const std::size_t width = cols + 2;
    std::vector<std::vector<T>> out;
    out.reserve(grid.size() + 2);
    out.emplace_back(width, fill);
    for (const auto& row : grid) {
        std::vector<T> framed(width, fill);
        for (std::size_t j = 0; j < row.size(); ++j) {
            framed[j + 1] = row[j];
        }
        out.push_back(std::move(framed));
    }
    out.emplace_back(width, fill);
    return out;
}

}  // namespace mt
