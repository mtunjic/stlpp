#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

namespace mt {

/// Output iterator that discards assignments and counts them. Copies share
/// the same counter (so `std::copy` sees the total).
///
/// Example:
///   mt::counting_output_iterator out;
///   std::copy(v.begin(), v.end(), out);
///   out.count();  // v.size()
class counting_output_iterator {
public:
    using iterator_concept = std::output_iterator_tag;
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    struct proxy {
        std::size_t* n = nullptr;
        template <typename T>
        const proxy& operator=(T&&) const
        {
            ++*n;
            return *this;
        }
    };

    counting_output_iterator() : n_(std::make_shared<std::size_t>(0)) {}

    [[nodiscard]] proxy operator*() const { return {n_.get()}; }
    counting_output_iterator& operator++() { return *this; }
    counting_output_iterator operator++(int) { return *this; }

    [[nodiscard]] std::size_t count() const { return *n_; }

private:
    std::shared_ptr<std::size_t> n_;
};

}  // namespace mt
