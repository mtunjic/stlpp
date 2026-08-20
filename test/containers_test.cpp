#include "stlpp.h"

#include <functional>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(Tape, InitList_ReadWriteAndMove) {
    mt::tape<int> t{1, 2, 3};
    EXPECT_EQ(t.size(), 3);
    EXPECT_EQ(t.position(), 0);
    EXPECT_EQ(t.read(), 1);

    t.right();
    t.write(9);
    EXPECT_EQ(t.read(), 9);
    EXPECT_EQ(std::vector<int>(t.begin(), t.end()), (std::vector<int>{1, 9, 3}));

    t.left();
    EXPECT_EQ(t.read(), 1);
}

TEST(Tape, LeftOffStart_GrowsDefaultCell) {
    mt::tape<int> t{7};
    t.left();
    EXPECT_EQ(t.size(), 2);
    EXPECT_EQ(t.position(), 0);
    EXPECT_EQ(t.read(), 0);
    t.right();
    EXPECT_EQ(t.read(), 7);
}

TEST(Tape, EmptyInitList_StillHasOneCell) {
    mt::tape<int> t{std::initializer_list<int>{}};
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.read(), 0);
}

TEST(Rope, ConcatAppendAndIndex) {
    mt::rope a{"hello "};
    mt::rope b{"world"};
    auto c = mt::concat(a, b);
    EXPECT_EQ(c.size(), 11);
    EXPECT_EQ(c.at(0), 'h');
    EXPECT_EQ(c.at(6), 'w');
    EXPECT_EQ(c.to_string(), "hello world");

    c.append("!");
    EXPECT_EQ(c.to_string(), "hello world!");
}

TEST(Rope, EmptyAndIndependentCopyShare) {
    mt::rope empty;
    EXPECT_TRUE(empty.empty());
    EXPECT_THROW((void)empty.at(0), std::out_of_range);
    EXPECT_EQ(mt::concat(empty, mt::rope{"x"}).to_string(), "x");

    mt::rope a{"ab"};
    mt::rope b = a;
    a.append("c");
    EXPECT_EQ(a.to_string(), "abc");
    EXPECT_EQ(b.to_string(), "ab");
}

TEST(ArraySlice, WindowAndSubClip) {
    std::vector<int> v{0, 1, 2, 3, 4};
    mt::array_slice<int> s{v.data() + 1, 3};
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s.back(), 3);

    auto clipped = s.sub(2, 99);
    EXPECT_EQ(clipped.size(), 1);
    EXPECT_EQ(clipped.front(), 3);

    mt::array_slice<int> whole{v};
    EXPECT_EQ(whole.size(), 5);

    mt::array_slice<int> clipped_ctor{v, 10, 3};
    EXPECT_TRUE(clipped_ctor.empty());
    EXPECT_THROW((void)clipped_ctor[0], std::out_of_range);
    EXPECT_THROW((void)s[3], std::out_of_range);
}

TEST(Htable, InsertFindOverwriteErase) {
    mt::htable<int, std::string> t;
    t.insert(3, "three");
    t.insert(1, "one");
    ASSERT_NE(t.find(3), nullptr);
    EXPECT_EQ(*t.find(3), "three");
    EXPECT_EQ(t.find(9), nullptr);
    EXPECT_TRUE(t.contains(3));

    EXPECT_FALSE(t.insert(3, "III"));
    EXPECT_EQ(*t.find(3), "three");
    EXPECT_FALSE(t.insert_or_assign(3, "III"));
    EXPECT_EQ(*t.find(3), "III");
    EXPECT_EQ(t.size(), 2);

    EXPECT_TRUE(t.erase(1));
    EXPECT_FALSE(t.erase(1));
    EXPECT_EQ(t.find(1), nullptr);
    EXPECT_EQ(*t.find(3), "III");
}

TEST(Htable, RehashAndTombReuse) {
    mt::htable<int, int> t;
    for (int i = 0; i < 20; ++i) {
        t.insert(i, i * 10);
    }
    EXPECT_EQ(t.size(), 20);
    for (int i = 0; i < 20; ++i) {
        ASSERT_NE(t.find(i), nullptr);
        EXPECT_EQ(*t.find(i), i * 10);
    }
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(t.erase(i));
    }
    t.insert(100, 1);
    ASSERT_NE(t.find(100), nullptr);
    EXPECT_EQ(*t.find(100), 1);
    EXPECT_EQ(*t.find(19), 190);
}

TEST(Htable, AcceptsNonDefaultConstructibleValue) {
    struct ND {
        int v;
        explicit ND(int x) : v(x) {}
    };
    mt::htable<int, ND> t;
    EXPECT_TRUE(t.insert(1, ND{7}));
    ASSERT_NE(t.find(1), nullptr);
    EXPECT_EQ(t.find(1)->v, 7);
    EXPECT_FALSE(t.insert(1, ND{8}));
    EXPECT_EQ(t.find(1)->v, 7);
}

TEST(VectorNd, TwoAndThreeDIndexing) {
    mt::vector_2d<int> m(2, 3, 0);
    m(1, 2) = 7;
    EXPECT_EQ(m.rows(), 2);
    EXPECT_EQ(m.cols(), 3);
    EXPECT_EQ(m(1, 2), 7);
    EXPECT_EQ(m(0, 0), 0);
    EXPECT_THROW((void)m(2, 0), std::out_of_range);
    EXPECT_THROW((void)mt::vector_2d<int>(std::numeric_limits<std::size_t>::max(), 2),
                 std::length_error);

    mt::vector_3d<int> v(2, 2, 2, 0);
    v(1, 0, 1) = 4;
    EXPECT_EQ(v.extent(0), 2);
    EXPECT_EQ(v(1, 0, 1), 4);
    EXPECT_EQ(v.size(), 8);
}

TEST(Grid, Neighbors4AtCornerAndCenter) {
    mt::grid<int> g(3, 3, 0);
    g(1, 1) = 1;
    auto center = g.neighbors4(1, 1);
    EXPECT_EQ(center.size(), 4);

    auto corner = g.neighbors4(0, 0);
    EXPECT_EQ(corner.size(), 2);
}

TEST(Graph, OutOfRangeVertexThrows) {
    mt::graph g(2);
    EXPECT_THROW(g.add_edge(0, 2), std::out_of_range);
    EXPECT_THROW((void)g.neighbors(9), std::out_of_range);
}

TEST(Graph, DirectedAndUndirectedEdges) {
    mt::graph g(3);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    EXPECT_EQ(g.size(), 3);
    EXPECT_EQ(g.neighbors(0), (std::vector<std::size_t>{1}));
    EXPECT_TRUE(g.neighbors(2).empty());

    const auto v = g.add_vertex();
    g.add_undirected_edge(2, v);
    EXPECT_EQ(g.neighbors(2), (std::vector<std::size_t>{v}));
    EXPECT_EQ(g.neighbors(v), (std::vector<std::size_t>{2}));
}

TEST(Quadtree, SplitKeepsEveryInsertedPoint) {
    mt::quadtree q({0, 0, 64, 64});
    for (int i = 0; i < 8; ++i) {
        EXPECT_TRUE(q.insert({static_cast<double>(i) + 0.5, 0.5}));
    }
    EXPECT_EQ(q.query({0, 0, 64, 64}).size(), 8u);
}

TEST(DynamicPQueue, MinHeapUpdateDecreaseKey) {
    mt::dynamic_pqueue<int, std::greater<int>> q;
    auto h5 = q.push(5);
    auto h3 = q.push(3);
    q.push(9);
    EXPECT_EQ(q.top(), 3);

    q.update(h5, 1);
    EXPECT_EQ(q.top(), 1);
    EXPECT_TRUE(q.contains(h5));

    q.pop();
    EXPECT_EQ(q.top(), 3);
    EXPECT_FALSE(q.contains(h5));
    EXPECT_TRUE(q.contains(h3));
}

TEST(DynamicPQueue, EmptyPopAndStaleUpdateAreSafe) {
    mt::dynamic_pqueue<int> q;
    q.pop();
    EXPECT_THROW((void)q.top(), std::out_of_range);
    q.update(99, 1);
    auto h = q.push(3);
    q.pop();
    q.update(h, 0);
    EXPECT_TRUE(q.empty());
}

TEST(DynamicPQueue, MaxHeapDefaultCompare) {
    mt::dynamic_pqueue<int> q;
    q.push(2);
    q.push(8);
    q.push(5);
    EXPECT_EQ(q.top(), 8);
    q.pop();
    EXPECT_EQ(q.top(), 5);
}

TEST(Quadtree, InsertAndRangeQuery) {
    mt::quadtree q({0, 0, 100, 100});
    EXPECT_TRUE(q.insert({10, 10}));
    EXPECT_TRUE(q.insert({50, 50}));
    EXPECT_TRUE(q.insert({90, 10}));
    EXPECT_FALSE(q.insert({100, 0}));

    auto hits = q.query({0, 0, 20, 20});
    ASSERT_EQ(hits.size(), 1);
    EXPECT_EQ(hits[0].x, 10);
    EXPECT_EQ(hits[0].y, 10);
}

TEST(Quadtree, SplitThenQuery) {
    mt::quadtree q({0, 0, 64, 64});
    for (int i = 0; i < 16; ++i) {
        EXPECT_TRUE(q.insert({static_cast<double>(i) + 0.5, static_cast<double>(i) + 0.5}));
    }
    auto hits = q.query({0, 0, 4, 4});
    EXPECT_EQ(hits.size(), 4);
}

TEST(TsAdaptors, StackQueuePriorityTryPop) {
    mt::ts_stack<int> s;
    EXPECT_FALSE(s.try_pop().has_value());
    s.push(1);
    s.push(2);
    EXPECT_EQ(s.try_pop(), 2);
    EXPECT_EQ(s.try_pop(), 1);
    EXPECT_TRUE(s.empty());

    mt::ts_queue<int> q;
    q.push(1);
    q.push(2);
    EXPECT_EQ(q.try_pop(), 1);
    EXPECT_EQ(q.try_pop(), 2);

    mt::ts_priority_queue<int> pq;
    pq.push(1);
    pq.push(3);
    pq.push(2);
    EXPECT_EQ(pq.try_pop(), 3);
    EXPECT_EQ(pq.try_pop(), 2);
    EXPECT_EQ(pq.try_pop(), 1);
}

TEST(Zipper, MoveInsertErase) {
    mt::zipper<int> z{std::vector<int>{1, 2, 3}};
    EXPECT_EQ(z.focus(), 1);
    EXPECT_TRUE(z.move_right());
    EXPECT_EQ(z.focus(), 2);
    z.insert_right(9);
    EXPECT_EQ(z.to_vector(), (std::vector<int>{1, 2, 9, 3}));
    EXPECT_TRUE(z.erase_focus());
    EXPECT_EQ(z.focus(), 9);
}

TEST(RingBuffer, ZeroCapacityDoesNotModuloZero) {
    mt::ring_buffer<int> r(0);
    EXPECT_TRUE(r.empty());
    EXPECT_FALSE(r.try_push(1));
    EXPECT_THROW((void)r[0], std::out_of_range);
}

TEST(RingBuffer, PushPopWrap) {
    mt::ring_buffer<int> r(2);
    EXPECT_TRUE(r.try_push(1));
    EXPECT_TRUE(r.try_push(2));
    EXPECT_FALSE(r.try_push(3));
    EXPECT_EQ(r.try_pop(), 1);
    EXPECT_TRUE(r.try_push(3));
    EXPECT_EQ(r[0], 2);
    EXPECT_EQ(r.try_pop(), 2);
    EXPECT_EQ(r.try_pop(), 3);
    EXPECT_FALSE(r.try_pop().has_value());
}

TEST(DisjointSet, UniteFindComponents) {
    mt::disjoint_set ds(4);
    EXPECT_EQ(ds.components(), 4u);
    EXPECT_TRUE(ds.unite(0, 1));
    EXPECT_FALSE(ds.unite(0, 1));
    ds.unite(2, 3);
    EXPECT_TRUE(ds.connected(0, 1));
    EXPECT_FALSE(ds.connected(0, 2));
    EXPECT_EQ(ds.component_size(0), 2u);
    ds.unite(1, 2);
    EXPECT_EQ(ds.components(), 1u);
    EXPECT_THROW((void)ds.find(4), std::out_of_range);
}
