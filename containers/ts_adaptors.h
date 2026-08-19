#pragma once

#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

namespace mt {

/// Mutex-protected stack. `try_pop` returns empty when the stack is empty.
///
/// Example:
///   mt::ts_stack<int> s;
///   s.push(1);
///   auto x = s.try_pop();  // 1
template <typename T>
class ts_stack {
    std::stack<T> s_;
    mutable std::mutex m_;

public:
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(m_);
        s_.push(std::move(value));
    }

    [[nodiscard]] std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m_);
        if (s_.empty()) {
            return std::nullopt;
        }
        T value = std::move(s_.top());
        s_.pop();
        return value;
    }

    [[nodiscard]] bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_);
        return s_.empty();
    }
};

/// Mutex-protected FIFO queue.
///
/// Example:
///   mt::ts_queue<int> q;
///   q.push(1);
///   q.try_pop();  // 1
template <typename T>
class ts_queue {
    std::queue<T> q_;
    mutable std::mutex m_;

public:
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(m_);
        q_.push(std::move(value));
    }

    [[nodiscard]] std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m_);
        if (q_.empty()) {
            return std::nullopt;
        }
        T value = std::move(q_.front());
        q_.pop();
        return value;
    }

    [[nodiscard]] bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_);
        return q_.empty();
    }
};

/// Mutex-protected priority queue. Default `std::less` is a max-heap.
///
/// Example:
///   mt::ts_priority_queue<int> q;
///   q.push(1);
///   q.push(3);
///   q.try_pop();  // 3
template <typename T, typename Container = std::vector<T>, typename Compare = std::less<T>>
class ts_priority_queue {
    std::priority_queue<T, Container, Compare> q_;
    mutable std::mutex m_;

public:
    void push(T value)
    {
        std::lock_guard<std::mutex> lock(m_);
        q_.push(std::move(value));
    }

    [[nodiscard]] std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m_);
        if (q_.empty()) {
            return std::nullopt;
        }
        T value = q_.top();
        q_.pop();
        return value;
    }

    [[nodiscard]] bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_);
        return q_.empty();
    }
};

}  // namespace mt
