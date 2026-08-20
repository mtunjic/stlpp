#include "stlpp.h"

#include "support/container_postconditions.h"
#include "support/eop_postconditions.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(ContainerGeneric, TapeReadWriteInvariants)
{
    mt::tape<int> t{1, 2, 3};
    mt::test::expect_tape_invariants(t);
    EXPECT_EQ(t.read(), 1);
    t.right();
    mt::test::expect_tape_invariants(t);
    t.write(9);
    EXPECT_EQ(std::vector<int>(t.begin(), t.end()), (std::vector<int>{1, 9, 3}));
}

TEST(ContainerGeneric, RopeConcatSizeLaw)
{
    mt::rope a{"ab"};
    mt::rope b{"cd"};
    auto c = mt::concat(a, b);
    EXPECT_EQ(c.size(), a.size() + b.size());
    EXPECT_EQ(c.to_string(), a.to_string() + b.to_string());
}

TEST(ContainerGeneric, RopeAppendExtends)
{
    mt::rope r{"hi"};
    const std::size_t before = r.size();
    r.append("!");
    EXPECT_EQ(r.size(), before + 1);
    EXPECT_EQ(r.at(before), '!');
}

TEST(ContainerGeneric, HtableInsertFindRoundtrip)
{
    mt::htable<int, std::string> t;
    mt::test::expect_htable_roundtrip(t, 3, std::string{"three"});
    mt::test::expect_htable_roundtrip(t, 1, std::string{"one"});
    EXPECT_EQ(t.size(), 2u);
}

TEST(ContainerGeneric, RingBufferFifoWhenCapacityAllows)
{
    mt::ring_buffer<int> r(3);
    EXPECT_TRUE(r.try_push(1));
    EXPECT_TRUE(r.try_push(2));
    EXPECT_EQ(r.try_pop(), 1);
    EXPECT_TRUE(r.try_push(3));
    EXPECT_EQ(r.try_pop(), 2);
    EXPECT_EQ(r.try_pop(), 3);
}

TEST(ContainerGeneric, DisjointSetUniteTransitive)
{
    mt::disjoint_set ds(5);
    ds.unite(0, 1);
    ds.unite(1, 2);
    EXPECT_TRUE(ds.connected(0, 2));
    EXPECT_FALSE(ds.connected(0, 3));
    EXPECT_EQ(ds.component_size(0), 3u);
}

TEST(ContainerGeneric, GraphNeighborsAfterEdge)
{
    mt::graph g(3);
    g.add_edge(0, 1);
    g.add_undirected_edge(1, 2);
    mt::test::expect_ranges_equal(g.neighbors(0), std::vector<std::size_t>{1});
    mt::test::expect_ranges_equal(g.neighbors(2), std::vector<std::size_t>{1});
}

TEST(ContainerGeneric, ArraySliceSubSizeLaw)
{
    std::vector<int> v{0, 1, 2, 3, 4};
    mt::array_slice<int> s{v};
    auto sub = s.sub(1, 99);
    EXPECT_EQ(sub.size(), 4u);
    EXPECT_EQ(sub[0], 1);
    EXPECT_THROW((void)sub[4], std::out_of_range);
}

TEST(ContainerGeneric, ZipperToVectorAfterInsert)
{
    mt::zipper<int> z{std::vector{1, 2, 3}};
    EXPECT_TRUE(z.move_right());
    z.insert_right(9);
    EXPECT_EQ(z.to_vector(), (std::vector<int>{1, 2, 9, 3}));
    EXPECT_TRUE(z.move_left());
    EXPECT_EQ(z.focus(), 1);
}

TEST(ContainerGeneric, DynamicPqueueMaxHeapPopOrder)
{
    mt::dynamic_pqueue<int> q;
    q.push(2);
    q.push(8);
    q.push(5);
    mt::test::expect_pqueue_pop_order(q, {8, 5, 2});
}

TEST(ContainerGeneric, DynamicPqueueMinHeapUpdate)
{
    mt::dynamic_pqueue<int, std::greater<int>> q;
    auto h5 = q.push(5);
    q.push(3);
    q.update(h5, 1);
    EXPECT_EQ(q.top(), 1);
}

TEST(ContainerGeneric, QuadtreeSplitPreservesAllPoints)
{
    mt::quadtree q({0, 0, 64, 64});
    for (int i = 0; i < 8; ++i) {
        EXPECT_TRUE(q.insert({static_cast<double>(i) + 0.5, 0.5}));
    }
    mt::test::expect_quadtree_preserves_inserted(q, {0, 0, 64, 64}, 8);
}

TEST(ContainerGeneric, Vector2dIndexingLaw)
{
    mt::vector_2d<int> m(2, 3, 0);
    m(1, 2) = 7;
    EXPECT_EQ(m.rows(), 2u);
    EXPECT_EQ(m.cols(), 3u);
    EXPECT_EQ(m(1, 2), 7);
    EXPECT_THROW((void)m(2, 0), std::out_of_range);
}

TEST(ContainerGeneric, GridNeighbors4CornerCount)
{
    mt::grid<int> g(3, 3, 0);
    EXPECT_EQ(g.neighbors4(0, 0).size(), 2u);
    EXPECT_EQ(g.neighbors4(1, 1).size(), 4u);
}

TEST(ContainerGeneric, Vector3dIndexingLaw)
{
    mt::vector_3d<int> v(2, 2, 2, 0);
    v(1, 0, 1) = 4;
    EXPECT_EQ(v.extent(0), 2u);
    EXPECT_EQ(v(1, 0, 1), 4);
    EXPECT_EQ(v.size(), 8u);
    EXPECT_THROW((void)v(2, 0, 0), std::out_of_range);
}

TEST(ContainerGeneric, TsStackLifo)
{
    mt::ts_stack<int> s;
    EXPECT_FALSE(s.try_pop().has_value());
    mt::test::expect_ts_stack_lifo(s, {1, 2, 3});
}

TEST(ContainerGeneric, TsQueueFifo)
{
    mt::ts_queue<int> q;
    mt::test::expect_ts_queue_fifo(q, {1, 2, 3});
}

TEST(ContainerGeneric, TsPriorityQueuePopOrder)
{
    mt::ts_priority_queue<int> pq;
    mt::test::expect_ts_priority_pop_order(pq, {1, 3, 2}, {3, 2, 1});
}

}  // namespace
