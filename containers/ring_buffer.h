#pragma once

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace mt {

/// Fixed-capacity circular buffer. `try_push` fails when full (does not overwrite).
///
/// Example:
///   mt::ring_buffer<int> r(2);
///   r.try_push(1);
///   r.try_push(2);
///   r.try_pop();  // 1
template <typename T>
class ring_buffer {
    std::vector<T> buf_;
    std::size_t head_ = 0;
    std::size_t size_ = 0;

    [[nodiscard]] std::size_t index(std::size_t i) const { return (head_ + i) % buf_.size(); }

public:
    explicit ring_buffer(std::size_t capacity) : buf_(capacity) {}

    [[nodiscard]] std::size_t capacity() const { return buf_.size(); }
    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] bool empty() const { return size_ == 0; }
    [[nodiscard]] bool full() const { return size_ == buf_.size(); }

    bool try_push(T value)
    {
        if (full() || buf_.empty()) {
            return false;
        }
        buf_[index(size_)] = std::move(value);
        ++size_;
        return true;
    }

    [[nodiscard]] std::optional<T> try_pop()
    {
        if (empty()) {
            return std::nullopt;
        }
        T value = std::move(buf_[head_]);
        head_ = (head_ + 1) % buf_.size();
        --size_;
        return value;
    }

    [[nodiscard]] T& operator[](std::size_t i) { return buf_[index(i)]; }
    [[nodiscard]] const T& operator[](std::size_t i) const { return buf_[index(i)]; }
};

}  // namespace mt
