#include "stlpp.h"

#include <vector>

#include <gtest/gtest.h>

namespace {

mt::btree<int> make_sample()
{
    mt::btree<int> t;
    const auto r = t.set_root(1);
    const auto l = t.add_left(r, 2);
    t.add_right(r, 3);
    t.add_left(l, 4);
    return t;
}

TEST(BifurcateGeneric, WeightAndHeightMatchRecursiveAndIterative) {
    auto t = make_sample();
    const auto c = t.root();
    EXPECT_EQ(mt::weight_recursive(c), 4u);
    EXPECT_EQ(mt::weight(c), 4u);
    EXPECT_EQ(mt::height_recursive(c), 3u);
    EXPECT_EQ(mt::height(c), 3u);
}

TEST(BifurcateGeneric, IsomorphicVsEquivalent) {
    auto a = make_sample();
    auto b = make_sample();
    EXPECT_TRUE(mt::bifurcate_isomorphic_nonempty(a.root(), b.root()));
    EXPECT_TRUE(mt::bifurcate_equal_nonempty(a.root(), b.root()));
    EXPECT_TRUE(mt::bifurcate_isomorphic(a.root(), b.root()));
    EXPECT_TRUE(mt::bifurcate_equal(a.root(), b.root()));
    mt::sink(b.root()) = 9;
    EXPECT_TRUE(mt::bifurcate_isomorphic_nonempty(a.root(), b.root()));
    EXPECT_FALSE(mt::bifurcate_equal_nonempty(a.root(), b.root()));
}

TEST(BifurcateGeneric, ReachableAndTraverseVisitCount) {
    auto t = make_sample();
    const auto root = t.root();
    EXPECT_TRUE(mt::reachable(root, mt::left_successor(mt::left_successor(root))));
    EXPECT_FALSE(mt::reachable(mt::left_successor(root), root));
    int visits = 0;
    (void)mt::traverse_nonempty(root, [&](mt::visit, auto) { ++visits; });
    EXPECT_EQ(visits, 12);  // 3 phases × 4 nodes
}

TEST(BifurcateGeneric, LexicographicalTreeOrder) {
    auto a = make_sample();
    auto b = make_sample();
    EXPECT_FALSE(mt::bifurcate_less(a.root(), b.root()));
    mt::sink(b.root()) = 0;
    EXPECT_FALSE(mt::bifurcate_less(a.root(), b.root()));
    EXPECT_TRUE(mt::bifurcate_less(b.root(), a.root()));
}

TEST(BifurcateGeneric, RotatingWeightAndPhasedTraverse) {
    auto t = make_sample();
    EXPECT_EQ(mt::weight_rotating(t.root()), 4u);
    std::vector<int> pre;
    (void)mt::traverse_phased_rotating(t.root(), 0, [&](auto c) { pre.push_back(mt::source(c)); });
    EXPECT_EQ(pre.size(), 4u);
}

TEST(BifurcateGeneric, EmptyTrees) {
    mt::btree_coord<int> e{};
    EXPECT_TRUE(mt::empty(e));
    EXPECT_EQ(mt::weight_recursive(e), 0u);
    EXPECT_TRUE(mt::bifurcate_isomorphic(e, e));
    EXPECT_TRUE(mt::bifurcate_equal(e, e));
}

}  // namespace
