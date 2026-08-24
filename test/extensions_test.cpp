#include "stlpp.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

TEST(Functors, VowelDigitLessThan)
{
    EXPECT_TRUE(mt::vowel{}('e'));
    EXPECT_FALSE(mt::vowel{}('s'));
    EXPECT_TRUE(mt::digit{}('3'));
    EXPECT_FALSE(mt::digit{}('x'));
    EXPECT_TRUE(mt::less_than<int>{5}(3));
    EXPECT_FALSE(mt::less_than<int>{5}(5));
}

TEST(Functors, OddEvenHalveModulo)
{
    EXPECT_TRUE(mt::is_odd(3));
    EXPECT_TRUE(mt::is_even(4));
    int n = 10;
    mt::halve_non_negative(n);
    EXPECT_EQ(n, 5);
    mt::modulo_multiply<int> op{5};
    EXPECT_EQ(op(3, 4), 2);
    EXPECT_EQ(mt::identity_element(op), 1);
}

TEST(Functors, ArithmeticAndNorm)
{
    EXPECT_EQ(mt::plus<int>{}(2, 3), 5);
    EXPECT_EQ(mt::multiplies<int>{}(2, 3), 6);
    EXPECT_EQ(mt::divides<int>{}(8, 2), 4);
    EXPECT_EQ(mt::divides<int>{}(8, 0), 8);
    EXPECT_EQ(mt::minimum<int>{}(3, 1), 1);
    EXPECT_EQ(mt::norm<int>{}(3, 4), 25);
}

TEST(VectorExt, KeepIfMatchesReadme)
{
    std::vector<int> v{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
    mt::keep_if(v, mt::less_than<int>{8});
    EXPECT_EQ(v, (std::vector<int>{0, 1, 4, 4, 0, 7, 7, 0}));
}

TEST(VectorExt, DistinctFastEraseDeleteIf)
{
    std::vector<int> v{3, 1, 3, 2, 1};
    mt::distinct(v);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));

    mt::fast_erase(v, 0);
    EXPECT_EQ(v.size(), 2u);

    std::vector<int> w{1, 2, 3, 4};
    mt::delete_if(w, [](int x) { return x % 2 == 0; });
    EXPECT_EQ(w, (std::vector<int>{1, 3}));
    mt::keep_if_not(w, [](int x) { return x == 1; });
    EXPECT_EQ(w, (std::vector<int>{3}));
}

TEST(VectorExt, ReadNumsFromStream)
{
    std::istringstream in{"10 20 30"};
    std::vector<int> v;
    mt::read_nums(in, v);
    EXPECT_EQ(v, (std::vector<int>{10, 20, 30}));
}

TEST(StringExt, TrimRightIfReadme)
{
    std::string s = "woow!!!!";
    mt::trim_right_if(s, '!');
    EXPECT_EQ(s, "woow");

    std::wstring ws = L"Awesome........";
    mt::trim_right_if(ws, L'.');
    EXPECT_EQ(ws, L"Awesome");

    std::string set = "';;;'yep..'';";
    mt::trim_right_if(set, mt::is_any_of(";,.'"));
    mt::trim_left_if(set, mt::is_any_of(";,.'"));
    EXPECT_EQ(set, "yep");

    std::string spaces{"trim me     "};
    mt::trim_right_if(spaces, mt::is_space);
    EXPECT_EQ(spaces, "trim me");
}

TEST(StringExt, CasePrefixSanitize)
{
    EXPECT_EQ(mt::to_lower("MARKO"), "marko");
    EXPECT_EQ(mt::to_upper("cpp"), "CPP");
    EXPECT_EQ(mt::to_title("hello world"), "Hello World");

    std::string uri{"https://www.google.com"};
    EXPECT_TRUE(mt::has_prefix(uri, "https://"));
    EXPECT_TRUE(mt::has_suffix(uri, ".com"));

    std::wstring bad{L"some,12<script>-1=1test29710-1---,.1888Osss"};
    mt::sanitize(bad);
    EXPECT_EQ(mt::digits_copy(bad), L"12112971011888");
    EXPECT_EQ(mt::letters_copy(bad), L"somescripttestosss");

    EXPECT_EQ(mt::floats_copy("1.5x2.25"), (std::vector<double>{1.5, 2.25}));
}

TEST(StringExt, PalindromeParseRepeat)
{
    EXPECT_TRUE(mt::is_palindrome("abba"));
    EXPECT_FALSE(mt::is_palindrome("abc"));
    EXPECT_TRUE(mt::is_digit("42"));
    EXPECT_TRUE(mt::is_alpha("Hi"));
    EXPECT_EQ(mt::to_int("12"), 12);
    EXPECT_THROW((void)mt::to_int("12x"), std::invalid_argument);
    EXPECT_THROW((void)mt::from_string<int>(""), std::invalid_argument);
    EXPECT_EQ(mt::repeat("ab", 3), "ababab");
    EXPECT_EQ(mt::repeat_char('x', 4), "xxxx");
    EXPECT_TRUE(mt::eq_ignore_case("AbC", "aBc"));
    EXPECT_EQ(mt::count("banana", 'a'), 3u);
}

TEST(StringExt, RemoveAndSanitizeCopy)
{
    std::string sr{"-1,2,3;name=marko$"};
    mt::remove_if(sr, [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; });
    EXPECT_EQ(sr, "-,,;name=marko$");

    EXPECT_EQ(mt::sanitize_copy("Hello, 12!"), "hello12");
    EXPECT_EQ(mt::strip_copy(std::string{"  hi  "}), "hi");
}

TEST(StringExt, SetdelimAndRandomString)
{
    std::istringstream in{"1,2,3"};
    mt::setdelim(in, ",");
    int a = 0;
    int b = 0;
    int c = 0;
    in >> a >> b >> c;
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 2);
    EXPECT_EQ(c, 3);

    auto rs = mt::random_string(16);
    EXPECT_EQ(rs.size(), 16u);
    EXPECT_TRUE(std::all_of(rs.begin(), rs.end(), [](char ch) {
        return std::isalnum(static_cast<unsigned char>(ch)) != 0;
    }));
}

TEST(StringExt, PadChopReplaceGsub)
{
    std::string n{"7"};
    mt::left_pad(n, 3, '0');
    EXPECT_EQ(n, "007");
    mt::right_pad(n, 5, '.');
    EXPECT_EQ(n, "007..");
    std::string mid{"hi"};
    mt::center(mid, 6, '-');
    EXPECT_EQ(mid, "--hi--");

    std::string ch{"hi!"};
    mt::chop(ch);
    EXPECT_EQ(ch, "hi");
    std::string nl{"line\r\n"};
    mt::chomp(nl);
    EXPECT_EQ(nl, "line");

    EXPECT_EQ(mt::replace_copy("aa ba", "a", "x"), "xa ba");
    EXPECT_EQ(mt::gsub_copy("aa ba", "a", "x"), "xx bx");
}

TEST(StringExt, ContainsPluralizeCrypt)
{
    EXPECT_TRUE(mt::contains("hello", "ell"));
    EXPECT_TRUE(mt::contains("hello", 'h'));
    EXPECT_FALSE(mt::contains_any("hello", "xz"));
    EXPECT_TRUE(mt::contains_any("hello", "xoz"));

    EXPECT_EQ(mt::pluralize("city"), "cities");
    EXPECT_EQ(mt::pluralize("box"), "boxes");
    EXPECT_EQ(mt::pluralize("day"), "days");
    EXPECT_EQ(mt::pluralize("cat"), "cats");
    EXPECT_EQ(mt::pluralize("dish"), "dishes");

    std::string s{"secret"};
    mt::crypt(s, "k");
    EXPECT_NE(s, "secret");
    mt::crypt(s, "k");
    EXPECT_EQ(s, "secret");
}

TEST(Utils, PrintPickRandomizeStopwatch)
{
    std::ostringstream out;
    mt::print(std::vector<int>{1, 2, 3}, out);
    EXPECT_EQ(out.str(), "[ 1, 2, 3 ]\n");

    const int n = mt::pick_a_number(3, 3);
    EXPECT_EQ(n, 3);

    std::vector<int> deck{0, 1, 2, 3, 4, 5, 6, 7};
    mt::randomize(deck);
    std::sort(deck.begin(), deck.end());
    EXPECT_EQ(deck, (std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7}));

    mt::stopwatch t;
    t.start();
    EXPECT_GE(t.stop(), 0.0);
}

TEST(Utils, ToMapListArrayDigitSumBench)
{
    auto m = mt::to_map(
        std::vector{std::pair{1, std::string{"a"}}, std::pair{2, std::string{"b"}},
                    std::pair{1, std::string{"A"}}});
    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m[1], "A");
    EXPECT_EQ(m[2], "b");

    auto lst = mt::to_list(std::vector{1, 2, 3});
    EXPECT_EQ(lst, (std::list<int>{1, 2, 3}));

    auto arr = mt::to_array<3>(std::vector{1, 2, 3, 4});
    EXPECT_EQ(arr, (std::array<int, 3>{1, 2, 3}));

    EXPECT_EQ(mt::digit_sum(123), 6);
    EXPECT_EQ(mt::digit_sum(-17), 8);
    EXPECT_EQ(mt::digit_sum("a1b2"), 3);

    EXPECT_GE(mt::bench([] {}), 0.0);
}

TEST(Utils, ThreadGuardJoins)
{
    std::atomic<int> n{0};
    std::thread worker{[&] { n = 1; }};
    {
        mt::thread_guard g{worker};
    }
    EXPECT_EQ(n.load(), 1);
    EXPECT_FALSE(worker.joinable());

    std::atomic<int> m{0};
    std::vector<std::thread> ts;
    ts.emplace_back([&] { m.fetch_add(1); });
    ts.emplace_back([&] { m.fetch_add(1); });
    {
        mt::join_threads j{ts};
    }
    EXPECT_EQ(m.load(), 2);
}
