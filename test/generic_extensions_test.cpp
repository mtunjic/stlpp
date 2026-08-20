#include "stlpp.h"

#include "support/eop_postconditions.h"

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

TEST(ExtensionsGeneric, SplitJoinRoundTripSingleDelimiter) {
    const std::string original{"a,b,c"};
    const auto parts = mt::split_copy(original, ",");
    EXPECT_EQ(mt::join(parts, ","), original);
}

TEST(ExtensionsGeneric, RepeatLengthLaw) {
    EXPECT_EQ(mt::repeat("ab", 0), "");
    EXPECT_EQ(mt::repeat("ab", 3).size(), 6u);
    EXPECT_EQ(mt::repeat_char('x', 5), "xxxxx");
}

TEST(ExtensionsGeneric, ReverseCopyInvolution) {
    const std::string s{"hello"};
    EXPECT_EQ(mt::reverse_copy(mt::reverse_copy(s)), s);
}

TEST(ExtensionsGeneric, ToLowerUpperAscii) {
    EXPECT_EQ(mt::to_lower("AbC"), "abc");
    EXPECT_EQ(mt::to_upper("AbC"), "ABC");
    EXPECT_TRUE(mt::eq_ignore_case("AbC", "aBc"));
}

TEST(ExtensionsGeneric, FromStringRoundTripInt) {
    EXPECT_EQ(mt::to_string(42), "42");
    EXPECT_EQ(mt::to_int("42"), 42);
}

TEST(ExtensionsGeneric, PalindromeLaw) {
    EXPECT_TRUE(mt::is_palindrome("abba"));
    EXPECT_FALSE(mt::is_palindrome("abc"));
    EXPECT_TRUE(mt::is_palindrome(""));
}

TEST(ExtensionsGeneric, CountMatchesManual) {
    const std::string s{"banana"};
    EXPECT_EQ(mt::count(s, 'a'), 3u);
}

TEST(ExtensionsGeneric, InvalidParseThrows) {
    EXPECT_THROW((void)mt::to_int("12x"), std::invalid_argument);
    EXPECT_THROW((void)mt::repeat("a", std::numeric_limits<std::size_t>::max()), std::length_error);
}

}  // namespace
