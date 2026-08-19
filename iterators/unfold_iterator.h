#pragma once

#include <functional>
#include <iterator>
#include <optional>
#include <utility>

namespace mt {

/// Input iterator over an `unfold` sequence. Default-constructed is the end.
///
/// Example:
///   auto f = [](int n) -> std::optional<std::pair<int, int>> {
///       if (n <= 0) return std::nullopt;
///       return std::pair{n, n - 1};
///   };
///   mt::unfold_iterator it{f, 3};
///   // *it == 3
template <typename F, typename State>
class unfold_iterator {
public:
    using Step = std::invoke_result_t<F&, const State&>;
    using Pair = typename Step::value_type;
    using iterator_concept = std::input_iterator_tag;
    using iterator_category = std::input_iterator_tag;
    using value_type = typename Pair::first_type;
    using difference_type = std::ptrdiff_t;
    using reference = value_type;
    using pointer = void;

    unfold_iterator() = default;
    unfold_iterator(F f, State state) : f_(std::move(f)), step_(std::invoke(f_, state)) {}

    [[nodiscard]] reference operator*() const { return step_->first; }

    unfold_iterator& operator++()
    {
        step_ = std::invoke(f_, step_->second);
        return *this;
    }

    unfold_iterator operator++(int)
    {
        unfold_iterator tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(const unfold_iterator& a, const unfold_iterator& b)
    {
        return static_cast<bool>(a.step_) == static_cast<bool>(b.step_) &&
               (!a.step_ || a.step_->second == b.step_->second);
    }

private:
    [[no_unique_address]] F f_{};
    Step step_{};
};

}  // namespace mt
