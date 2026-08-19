#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <utility>
#include <vector>

namespace mt {

/// Huet zipper on a sequence: focus plus O(1) local move/insert/delete.
///
/// Example:
///   mt::zipper<int> z{std::vector{1, 2, 3}};
///   z.focus();      // 1
///   z.move_right();
///   z.focus();      // 2
template <typename T>
class zipper {
    std::deque<T> left_;
    std::optional<T> focus_;
    std::deque<T> right_;

public:
    zipper() = default;

    explicit zipper(std::vector<T> values)
    {
        if (values.empty()) {
            return;
        }
        focus_ = std::move(values.front());
        for (std::size_t i = 1; i < values.size(); ++i) {
            right_.push_back(std::move(values[i]));
        }
    }

    [[nodiscard]] bool empty() const { return !focus_; }
    [[nodiscard]] bool at_left() const { return left_.empty(); }
    [[nodiscard]] bool at_right() const { return right_.empty(); }

    [[nodiscard]] T& focus() { return *focus_; }
    [[nodiscard]] const T& focus() const { return *focus_; }

    bool move_right()
    {
        if (!focus_ || right_.empty()) {
            return false;
        }
        left_.push_back(std::move(*focus_));
        focus_ = std::move(right_.front());
        right_.pop_front();
        return true;
    }

    bool move_left()
    {
        if (!focus_ || left_.empty()) {
            return false;
        }
        right_.push_front(std::move(*focus_));
        focus_ = std::move(left_.back());
        left_.pop_back();
        return true;
    }

    void insert_right(T value)
    {
        if (!focus_) {
            focus_ = std::move(value);
            return;
        }
        right_.push_front(std::move(value));
    }

    bool erase_focus()
    {
        if (!focus_) {
            return false;
        }
        if (!right_.empty()) {
            focus_ = std::move(right_.front());
            right_.pop_front();
            return true;
        }
        if (!left_.empty()) {
            focus_ = std::move(left_.back());
            left_.pop_back();
            return true;
        }
        focus_.reset();
        return true;
    }

    [[nodiscard]] std::vector<T> to_vector() const
    {
        std::vector<T> out;
        out.insert(out.end(), left_.begin(), left_.end());
        if (focus_) {
            out.push_back(*focus_);
        }
        out.insert(out.end(), right_.begin(), right_.end());
        return out;
    }
};

}  // namespace mt
