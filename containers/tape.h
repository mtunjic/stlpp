#pragma once

#include <cstddef>
#include <deque>
#include <initializer_list>
#include <utility>

namespace mt {

/// Bidirectional tape with a movable head. Moving off an end grows a
/// default-constructed cell (Turing-tape style).
///
/// Example:
///   mt::tape<int> t{1, 2, 3};
///   t.right();
///   t.write(9);   // {1, 9, 3}, head on 9
///   t.left();
///   t.read();     // 1
template <typename T>
class tape {
    std::deque<T> cells_;
    std::size_t head_ = 0;

public:
    using iterator = typename std::deque<T>::iterator;
    using const_iterator = typename std::deque<T>::const_iterator;

    tape() : cells_(1), head_(0) {}

    tape(std::initializer_list<T> values) : cells_(values), head_(0)
    {
        if (cells_.empty()) {
            cells_.emplace_back();
        }
    }

    [[nodiscard]] std::size_t size() const { return cells_.size(); }
    [[nodiscard]] std::size_t position() const { return head_; }

    [[nodiscard]] T& read() { return cells_[head_]; }
    [[nodiscard]] const T& read() const { return cells_[head_]; }

    void write(T value) { cells_[head_] = std::move(value); }

    void right()
    {
        if (head_ + 1 == cells_.size()) {
            cells_.emplace_back();
        }
        ++head_;
    }

    void left()
    {
        if (head_ == 0) {
            cells_.emplace_front();
        } else {
            --head_;
        }
    }

    [[nodiscard]] iterator begin() { return cells_.begin(); }
    [[nodiscard]] iterator end() { return cells_.end(); }
    [[nodiscard]] const_iterator begin() const { return cells_.begin(); }
    [[nodiscard]] const_iterator end() const { return cells_.end(); }
};

}  // namespace mt
