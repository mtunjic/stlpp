#include "stlpp.h"

#include "support/test_concepts.h"

#include <string>

#include <gtest/gtest.h>

static_assert(std::random_access_iterator<mt::numeric_iterator<int>>);
static_assert(std::forward_iterator<mt::step_iterator<int*>>);
static_assert(std::forward_iterator<mt::edge_iterator<int*>>);
static_assert(std::input_iterator<mt::transform_iterator<int*, int (*)(int)>>);
static_assert(std::forward_iterator<mt::filter_iterator<int*, bool (*)(int)>>);
static_assert(std::input_iterator<mt::zip_iterator<int*, int*>>);
static_assert(std::forward_iterator<mt::cycle_iterator<std::vector<int>::iterator>>);
static_assert(std::forward_iterator<mt::window_iterator<std::vector<int>::iterator>>);
static_assert(std::forward_iterator<mt::chunk_iterator<std::vector<int>::iterator>>);
static_assert(std::input_iterator<mt::transform_iterator<int*, int (*)(int)>>);
static_assert(std::output_iterator<mt::counting_output_iterator, int>);

static_assert(mt::Regular<int>);
static_assert(mt::Regular<std::string>);
static_assert(mt::Binary_operation<std::plus<int>, int>);
static_assert(mt::Binary_operation<std::multiplies<int>, int>);

TEST(ConceptAxioms, RegularInt) {
    mt::test::check_regular_axioms(42);
}

TEST(ConceptAxioms, RegularString) {
    mt::test::check_regular_axioms(std::string{"hello"});
}

TEST(ConceptAxioms, AssociativePlusSpotCheck) {
    mt::test::check_associative_spot(std::plus<int>{}, 1, 2, 3);
    mt::test::check_associative_spot(std::plus<int>{}, -5, 10, 7);
}

TEST(ConceptAxioms, AssociativeMultipliesSpotCheck) {
    mt::test::check_associative_spot(std::multiplies<int>{}, 2, 3, 5);
}

TEST(ConceptAxioms, IteratorBasisCountingAdapter) {
    struct counting_basis {
        using state_type = int;
        using value_type = int;
        using reference = int;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        int s = 0;
        [[nodiscard]] int state() const { return s; }
        [[nodiscard]] int deref() const { return s; }
        void increment() { ++s; }
        void decrement() { --s; }
        void offset(std::ptrdiff_t n) { s += static_cast<int>(n); }
        [[nodiscard]] std::ptrdiff_t difference(const counting_basis& o) const { return s - o.s; }
    };
    static_assert(mt::IteratorBasis<counting_basis>);
    static_assert(mt::RandomAccessIteratorBasis<counting_basis>);
}
