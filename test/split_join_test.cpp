#include "split.h"

#include <list>
#include <string>
#include <vector>

#include <gtest/gtest.h>

TEST(Split, ReadmeSemicolon_KeepsCommaGroups)
{
    const std::string line{"man,meal,moon;fat,food,feel;cat,coat,cook;love,leg,lunch"};
    std::vector<std::string> words;
    mt::split(line, ";", words);

    ASSERT_EQ(words.size(), 4u);
    EXPECT_EQ(words[0], "man,meal,moon");
    EXPECT_EQ(words[1], "fat,food,feel");
    EXPECT_EQ(words[2], "cat,coat,cook");
    EXPECT_EQ(words[3], "love,leg,lunch");
}

TEST(Split, ReadmeCharSet_SplitsOnAnyDelimiter)
{
    const std::string line{"man,meal,moon;fat,food,feel;cat,coat,cook;love,leg,lunch"};
    std::list<std::string> words;
    mt::split(line, ",;.", words);

    const std::vector<std::string> expected{"man", "meal", "moon", "fat",  "food", "feel",
                                            "cat", "coat", "cook", "love", "leg",  "lunch"};
    EXPECT_EQ(std::vector<std::string>(words.begin(), words.end()), expected);
}

TEST(Split, EmptyInput_EmitsOneEmptyToken)
{
    std::vector<std::string> words;
    mt::split("", ";", words);
    ASSERT_EQ(words.size(), 1u);
    EXPECT_TRUE(words.front().empty());
}

TEST(Split, NoDelimiter_EmitsWholeString)
{
    std::vector<std::string> words;
    mt::split("alpha", ";", words);
    ASSERT_EQ(words.size(), 1u);
    EXPECT_EQ(words.front(), "alpha");
}

TEST(Split, ConsecutiveDelimiters_EmitEmptyTokens)
{
    std::vector<std::string> words;
    mt::split("a;;b", ";", words);
    ASSERT_EQ(words.size(), 3u);
    EXPECT_EQ(words[0], "a");
    EXPECT_TRUE(words[1].empty());
    EXPECT_EQ(words[2], "b");
}

TEST(Split, LeadingAndTrailingDelimiter_EmitsEmptyEdges)
{
    std::vector<std::string> words;
    mt::split(";a;", ";", words);
    ASSERT_EQ(words.size(), 3u);
    EXPECT_TRUE(words[0].empty());
    EXPECT_EQ(words[1], "a");
    EXPECT_TRUE(words[2].empty());
}

TEST(Split, SingleCharDelimiter_MatchesStringViewOverload)
{
    std::vector<std::string> words;
    mt::split("x-y-z", '-', words);
    const auto copied = mt::split_copy("x-y-z", "-");
    EXPECT_EQ(words, copied);
}

TEST(Join, EmptyRange_ReturnsEmptyString)
{
    const std::vector<std::string> parts;
    EXPECT_TRUE(mt::join(parts, ",").empty());
}

TEST(Join, SinglePart_OmitsSeparator)
{
    const std::vector<std::string> parts{"only"};
    EXPECT_EQ(mt::join(parts, ","), "only");
}

TEST(Join, ManyParts_InsertsSeparator)
{
    const std::vector<std::string> parts{"a", "b", "c"};
    EXPECT_EQ(mt::join(parts, ","), "a,b,c");
}

TEST(Join, RoundTripWithSingleCharDelimiter)
{
    const std::string src{"a;;b;c;"};
    const auto parts = mt::split_copy(src, ";");
    EXPECT_EQ(mt::join(parts, ";"), src);
}

TEST(JoinCopy, MatchesJoin)
{
    const std::vector<std::string> parts{"man", "meal", "moon"};
    EXPECT_EQ(mt::join_copy(parts, ","), mt::join(parts, ","));
}

TEST(Split, WideString_SplitsOnWideDelimiter)
{
    std::vector<std::wstring> words;
    mt::split(std::wstring{L"one|two"}, std::wstring{L"|"}, words);
    ASSERT_EQ(words.size(), 2u);
    EXPECT_EQ(words[0], L"one");
    EXPECT_EQ(words[1], L"two");
    EXPECT_EQ(mt::join(words, std::wstring_view{L"-"}), L"one-two");
}
