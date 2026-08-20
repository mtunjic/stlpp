#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mt {

namespace detail {

[[nodiscard]] inline std::size_t mul_extent(std::size_t a, std::size_t b)
{
    if (b != 0 && a > std::numeric_limits<std::size_t>::max() / b) {
        throw std::length_error("vector_nd size overflow");
    }
    return a * b;
}

}  // namespace detail

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
        : rows_(rows), cols_(cols), data_(detail::mul_extent(rows, cols), std::move(fill))
    {}

    [[nodiscard]] std::size_t rows() const { return rows_; }
    [[nodiscard]] std::size_t cols() const { return cols_; }
    [[nodiscard]] std::size_t size() const { return data_.size(); }

    [[nodiscard]] T& operator()(std::size_t r, std::size_t c)
    {
        check(r, c);
        return data_[index(r, c)];
    }
    [[nodiscard]] const T& operator()(std::size_t r, std::size_t c) const
    {
        check(r, c);
        return data_[index(r, c)];
    }

    [[nodiscard]] T* data() { return data_.data(); }
    [[nodiscard]] const T* data() const { return data_.data(); }

    [[nodiscard]] auto begin() { return data_.begin(); }
    [[nodiscard]] auto end() { return data_.end(); }
    [[nodiscard]] auto begin() const { return data_.begin(); }
    [[nodiscard]] auto end() const { return data_.end(); }

private:
    void check(std::size_t r, std::size_t c) const
    {
        if (r >= rows_ || c >= cols_) {
            throw std::out_of_range("vector_2d index");
        }
    }
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
        : d0_(d0), d1_(d1), d2_(d2), data_(detail::mul_extent(detail::mul_extent(d0, d1), d2), std::move(fill))
    {}

    [[nodiscard]] std::size_t extent(std::size_t dim) const
    {
        return dim == 0 ? d0_ : (dim == 1 ? d1_ : d2_);
    }
    [[nodiscard]] std::size_t size() const { return data_.size(); }

    [[nodiscard]] T& operator()(std::size_t i, std::size_t j, std::size_t k)
    {
        check(i, j, k);
        return data_[index(i, j, k)];
    }
    [[nodiscard]] const T& operator()(std::size_t i, std::size_t j, std::size_t k) const
    {
        check(i, j, k);
        return data_[index(i, j, k)];
    }

    [[nodiscard]] auto begin() { return data_.begin(); }
    [[nodiscard]] auto end() { return data_.end(); }
    [[nodiscard]] auto begin() const { return data_.begin(); }
    [[nodiscard]] auto end() const { return data_.end(); }

private:
    void check(std::size_t i, std::size_t j, std::size_t k) const
    {
        if (i >= d0_ || j >= d1_ || k >= d2_) {
            throw std::out_of_range("vector_3d index");
        }
    }
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
        std::vector<std::pair<std::size_t, std::size_t>> out;
        if (r < rows() && c < cols()) {
            if (r > 0) {
                out.emplace_back(r - 1, c);
            }
            if (r + 1 < rows()) {
                out.emplace_back(r + 1, c);
            }
            if (c > 0) {
                out.emplace_back(r, c - 1);
            }
            if (c + 1 < cols()) {
                out.emplace_back(r, c + 1);
            }
        }
        return out;
    }
};

}  // namespace mt
