#pragma once

#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

namespace mt {

/// Non-owning contiguous window (the Slice container). Distinct from the
/// `slice` algorithm, which returns an iterator pair.
///
/// Example:
///   std::vector<int> v{0, 1, 2, 3, 4};
///   mt::array_slice<int> s{v.data() + 1, 3};
///   s[0];          // 1
///   s.size();      // 3
template <typename T>
class array_slice {
    T* data_ = nullptr;
    std::size_t size_ = 0;

public:
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    array_slice() = default;
    array_slice(T* data, std::size_t n) : data_(data), size_(data == nullptr ? 0 : n) {}
    array_slice(std::span<T> s) : data_(s.data()), size_(s.size()) {}

    template <std::size_t N>
    explicit array_slice(T (&arr)[N]) : data_(arr),
                                        size_(N)
    {}

    array_slice(std::vector<T>& v) : data_(v.data()), size_(v.size()) {}
    array_slice(std::vector<T>& v, std::size_t start, std::size_t count)
    {
        if (start > v.size()) {
            start = v.size();
        }
        if (count > v.size() - start) {
            count = v.size() - start;
        }
        data_ = v.data() + start;
        size_ = count;
    }

    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }

    [[nodiscard]] T& operator[](std::size_t i)
    {
        check(i);
        return data_[i];
    }
    [[nodiscard]] const T& operator[](std::size_t i) const
    {
        check(i);
        return data_[i];
    }

    [[nodiscard]] T& front() { return (*this)[0]; }
    [[nodiscard]] const T& front() const { return (*this)[0]; }
    [[nodiscard]] T& back() { return (*this)[size_ - 1]; }
    [[nodiscard]] const T& back() const { return (*this)[size_ - 1]; }

    [[nodiscard]] T* data() { return data_; }
    [[nodiscard]] const T* data() const { return data_; }

    [[nodiscard]] iterator begin() { return data_; }
    [[nodiscard]] iterator end() { return data_ + size_; }
    [[nodiscard]] const_iterator begin() const { return data_; }
    [[nodiscard]] const_iterator end() const { return data_ + size_; }

    [[nodiscard]] array_slice sub(std::size_t start, std::size_t count) const
    {
        if (start > size_) {
            start = size_;
        }
        if (count > size_ - start) {
            count = size_ - start;
        }
        return {data_ + start, count};
    }

private:
    void check(std::size_t i) const
    {
        if (i >= size_ || data_ == nullptr) {
            throw std::out_of_range("array_slice index");
        }
    }
};

}  // namespace mt
