#include "stlpp.h"

#include "support/fixtures.h"
#include "support/test_concepts.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <gtest/gtest.h>

namespace {

template <typename Container>
class CopyBackwardGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(CopyBackwardGeneric, mt::test::EopCoreContainers);

TYPED_TEST(CopyBackwardGeneric, RoundTripViaBufferMatchesSource)
{
    constexpr int n = 6;
    auto src = mt::test::make_iota_container<TypeParam>(n);
    const auto src_last = mt::test::filled_end(src, n);
    std::vector<int> dst(static_cast<std::size_t>(n), -1);
    const auto written = mt::copy_backward(src.begin(), src_last, dst.end());
    EXPECT_EQ(written, dst.begin());
    EXPECT_EQ(dst, mt::test::to_filled_vector(src, n));
}

TYPED_TEST(CopyBackwardGeneric, CountedCopyBackwardNMatchesBounded)
{
    constexpr int n = 5;
    auto src = mt::test::make_iota_container<TypeParam>(n);
    const auto src_last = mt::test::filled_end(src, n);
    std::vector<int> a(static_cast<std::size_t>(n), -1);
    std::vector<int> b(static_cast<std::size_t>(n), -1);
    const auto [in, out] = mt::copy_backward_n(src_last, n, a.end());
    EXPECT_EQ(in, src.begin());
    EXPECT_EQ(out, a.begin());
    (void)mt::copy_backward(src.begin(), src_last, b.end());
    EXPECT_EQ(a, b);
}

TEST(ReverseCopyBackwardGeneric, WritesReversedIntoTail)
{
    std::vector<int> src{1, 2, 3, 4};
    std::vector<int> dst(4, -1);
    const auto begin = mt::reverse_copy_backward(src.begin(), src.end(), dst.end());
    EXPECT_EQ(begin, dst.begin());
    EXPECT_EQ(dst, (std::vector<int>{4, 3, 2, 1}));
}

template <typename Container>
class RotateGeneric : public ::testing::Test {};

TYPED_TEST_SUITE(RotateGeneric, mt::test::EopCoreContainers);

TYPED_TEST(RotateGeneric, PreservesMultisetAndReturnsOldFirst)
{
    constexpr int n = 6;
    auto c = mt::test::make_iota_container<TypeParam>(n);
    const auto last = mt::test::filled_end(c, n);
    const auto before = mt::test::to_filled_vector(c, n);
    auto middle = c.begin();
    std::advance(middle, 2);
    const auto p = mt::rotate(c.begin(), middle, last);
    auto expected = before;
    std::rotate(expected.begin(), expected.begin() + 2, expected.end());
    EXPECT_EQ(mt::test::to_filled_vector(c, n), expected);
    EXPECT_EQ(std::distance(c.begin(), p), static_cast<int>(before.size()) - 2);
    EXPECT_EQ(*p, before.front());
}

TYPED_TEST(RotateGeneric, DoubleRotateRestoresWhenMiddleIsSymmetric)
{
    // For length 6, rotate by 3 twice restores.
    constexpr int n = 6;
    auto c = mt::test::make_iota_container<TypeParam>(n);
    const auto last = mt::test::filled_end(c, n);
    const auto original = mt::test::to_filled_vector(c, n);
    auto mid = c.begin();
    std::advance(mid, 3);
    (void)mt::rotate(c.begin(), mid, last);
    mid = c.begin();
    std::advance(mid, 3);
    (void)mt::rotate(c.begin(), mid, last);
    EXPECT_EQ(mt::test::to_filled_vector(c, n), original);
}

TEST(RotateGeneric, BufferVariantMatchesRotate)
{
    std::vector<int> a{0, 1, 2, 3, 4, 5};
    std::vector<int> b = a;
    const auto pa = mt::rotate(a.begin(), a.begin() + 2, a.end());
    const auto pb = mt::rotate_with_buffer(b.begin(), b.begin() + 2, b.end());
    EXPECT_EQ(a, b);
    EXPECT_EQ(std::distance(a.begin(), pa), std::distance(b.begin(), pb));
}

TEST(RotateGeneric, TrivialMiddlesAreIdentity)
{
    std::vector<int> v{1, 2, 3};
    EXPECT_EQ(mt::rotate(v.begin(), v.begin(), v.end()), v.end());
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(mt::rotate(v.begin(), v.end(), v.end()), v.begin());
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TEST(CopyBackwardOverlap, RightShiftUsesCopyBackward)
{
    std::vector<int> v{1, 2, 3, 4, 0, 0};
    (void)mt::copy_backward(v.begin(), v.begin() + 4, v.end());
    EXPECT_EQ(v, (std::vector<int>{1, 2, 1, 2, 3, 4}));
}

}  // namespace
