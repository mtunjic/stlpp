#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace mt {

/// Transpose a rectangular matrix stored as `rows` vectors of equal length.
/// The longest row sets the output width; missing cells are value-initialized.
///
/// Example:
///   std::vector<std::vector<int>> m{{1, 2, 3}, {4, 5, 6}};
///   auto t = mt::transpose(m);  // {{1, 4}, {2, 5}, {3, 6}}
template <typename T>
[[nodiscard]] std::vector<std::vector<T>> transpose(const std::vector<std::vector<T>>& matrix)
{
    if (matrix.empty()) {
        return {};
    }
    std::size_t cols = 0;
    for (const auto& row : matrix) {
        cols = std::max(cols, row.size());
    }
    std::vector<std::vector<T>> out(cols, std::vector<T>(matrix.size()));
    for (std::size_t i = 0; i < matrix.size(); ++i) {
        for (std::size_t j = 0; j < matrix[i].size(); ++j) {
            out[j][i] = matrix[i][j];
        }
    }
    return out;
}

}  // namespace mt
