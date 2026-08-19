#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace mt {

/// Dense row-major 2-D array.
///
/// Example:
///   mt::vector_2d<int> m(2, 3, 0);
///   m(1, 2) = 7;
///   m.rows();  // 2
template <typename T>
class vector_2d {
    std::size_t rows_ = 0;
    std::size_t cols_ = 0;
    std::vector<T> data_;

    [[nodiscard]] std::size_t index(std::size_t r, std::size_t c) const { return r * cols_ + c; }

public:
    vector_2d() = default;
    vector_2d(std::size_t rows, std::size_t cols, T fill = T{})
        : rows_(rows), cols_(cols), data_(rows * cols, std::move(fill))
    {}

    [[nodiscard]] std::size_t rows() const { return rows_; }
    [[nodiscard]] std::size_t cols() const { return cols_; }
    [[nodiscard]] std::size_t size() const { return data_.size(); }

    [[nodiscard]] T& operator()(std::size_t r, std::size_t c) { return data_[index(r, c)]; }
    [[nodiscard]] const T& operator()(std::size_t r, std::size_t c) const { return data_[index(r, c)]; }

    [[nodiscard]] T* data() { return data_.data(); }
    [[nodiscard]] const T* data() const { return data_.data(); }

    [[nodiscard]] auto begin() { return data_.begin(); }
    [[nodiscard]] auto end() { return data_.end(); }
    [[nodiscard]] auto begin() const { return data_.begin(); }
    [[nodiscard]] auto end() const { return data_.end(); }
};

/// Dense row-major 3-D array. Layout is `i * (d1*d2) + j * d2 + k`.
///
/// Example:
///   mt::vector_3d<int> v(2, 2, 2, 0);
///   v(1, 0, 1) = 4;
template <typename T>
class vector_3d {
    std::size_t d0_ = 0;
    std::size_t d1_ = 0;
    std::size_t d2_ = 0;
    std::vector<T> data_;

    [[nodiscard]] std::size_t index(std::size_t i, std::size_t j, std::size_t k) const
    {
        return (i * d1_ + j) * d2_ + k;
    }

public:
    vector_3d() = default;
    vector_3d(std::size_t d0, std::size_t d1, std::size_t d2, T fill = T{})
        : d0_(d0), d1_(d1), d2_(d2), data_(d0 * d1 * d2, std::move(fill))
    {}

    [[nodiscard]] std::size_t extent(std::size_t dim) const
    {
        return dim == 0 ? d0_ : (dim == 1 ? d1_ : d2_);
    }
    [[nodiscard]] std::size_t size() const { return data_.size(); }

    [[nodiscard]] T& operator()(std::size_t i, std::size_t j, std::size_t k)
    {
        return data_[index(i, j, k)];
    }
    [[nodiscard]] const T& operator()(std::size_t i, std::size_t j, std::size_t k) const
    {
        return data_[index(i, j, k)];
    }

    [[nodiscard]] auto begin() { return data_.begin(); }
    [[nodiscard]] auto end() { return data_.end(); }
    [[nodiscard]] auto begin() const { return data_.begin(); }
    [[nodiscard]] auto end() const { return data_.end(); }
};

/// 2-D grid with 4-neighbour walks. Storage is a `vector_2d`.
///
/// Example:
///   mt::grid<int> g(3, 3, 0);
///   g(1, 1) = 1;
///   for (auto [r, c] : g.neighbors4(1, 1)) { ... }
template <typename T>
class grid {
    vector_2d<T> cells_;

public:
    grid() = default;
    grid(std::size_t rows, std::size_t cols, T fill = T{}) : cells_(rows, cols, std::move(fill)) {}

    [[nodiscard]] std::size_t rows() const { return cells_.rows(); }
    [[nodiscard]] std::size_t cols() const { return cells_.cols(); }

    [[nodiscard]] T& operator()(std::size_t r, std::size_t c) { return cells_(r, c); }
    [[nodiscard]] const T& operator()(std::size_t r, std::size_t c) const { return cells_(r, c); }

    [[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>>
    neighbors4(std::size_t r, std::size_t c) const
    {
        static constexpr int dr[] = {-1, 1, 0, 0};
        static constexpr int dc[] = {0, 0, -1, 1};
        std::vector<std::pair<std::size_t, std::size_t>> out;
        for (int k = 0; k < 4; ++k) {
            const int nr = static_cast<int>(r) + dr[k];
            const int nc = static_cast<int>(c) + dc[k];
            if (nr >= 0 && nc >= 0 && static_cast<std::size_t>(nr) < rows() &&
                static_cast<std::size_t>(nc) < cols()) {
                out.emplace_back(static_cast<std::size_t>(nr), static_cast<std::size_t>(nc));
            }
        }
        return out;
    }
};

}  // namespace mt
