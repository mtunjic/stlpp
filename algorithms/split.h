#pragma once

#include "concepts.h"

#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace mt {

// split / join
//
// Flow (split): scan each character; if it is in the delimiter set, emit the
// current token and start a new one; at the end emit the last token. Consecutive
// delimiters produce empty tokens so that join(split(s, d), d) round-trips for
// a single-character delimiter.
//
// Flow (join): write the first part, then for each remaining part write
// separator then part.
//
// Iterator strength: input. Complexity: O(n) in the number of characters /
// parts. Delimiter test is O(|delims|) via string_view::find.
//
// Example:
//   std::string line{"man,meal,moon;fat,food,feel"};
//   std::vector<std::string> groups;
//   mt::split(line, ";", groups);          // {"man,meal,moon", "fat,food,feel"}
//   mt::join(groups, "|");                 // "man,meal,moon|fat,food,feel"

template <Input_iterator I, typename Out, Character Char = Value_type<I>>
    requires std::output_iterator<Out, std::basic_string<Char>>
Out split(I first, I last, std::basic_string_view<Char> delims, Out dest)
{
    std::basic_string<Char> token;
    for (; first != last; ++first) {
        if (delims.find(*first) != std::basic_string_view<Char>::npos) {
            *dest = std::move(token);
            ++dest;
            token.clear();
        } else {
            token.push_back(*first);
        }
    }
    *dest = std::move(token);
    ++dest;
    return dest;
}

template <typename Container>
void split(std::string_view input, std::string_view delims, Container& dest)
{
    split(input.begin(), input.end(), delims, std::back_inserter(dest));
}

template <typename Container>
void split(std::string_view input, char delim, Container& dest)
{
    split(input, std::string_view(&delim, 1), dest);
}

template <typename Container>
void split(std::wstring_view input, std::wstring_view delims, Container& dest)
{
    split(input.begin(), input.end(), delims, std::back_inserter(dest));
}

template <typename Container>
void split(std::wstring_view input, wchar_t delim, Container& dest)
{
    split(input, std::wstring_view(&delim, 1), dest);
}

[[nodiscard]] inline std::vector<std::string> split_copy(std::string_view input,
                                                         std::string_view delims)
{
    std::vector<std::string> tokens;
    split(input, delims, tokens);
    return tokens;
}

[[nodiscard]] inline std::vector<std::wstring> split_copy(std::wstring_view input,
                                                          std::wstring_view delims)
{
    std::vector<std::wstring> tokens;
    split(input, delims, tokens);
    return tokens;
}

template <Input_iterator I, Character Char>
    requires std::convertible_to<Value_type<I>, std::basic_string_view<Char>>
[[nodiscard]] std::basic_string<Char> join(I first, I last, std::basic_string_view<Char> sep)
{
    std::basic_string<Char> result;
    if (first == last) {
        return result;
    }

    result.append(static_cast<std::basic_string_view<Char>>(*first));
    ++first;
    for (; first != last; ++first) {
        result.append(sep);
        result.append(static_cast<std::basic_string_view<Char>>(*first));
    }
    return result;
}

template <std::ranges::input_range R>
[[nodiscard]] std::string join(const R& parts, std::string_view sep)
{
    return join(std::ranges::begin(parts), std::ranges::end(parts), sep);
}

template <std::ranges::input_range R>
[[nodiscard]] std::wstring join(const R& parts, std::wstring_view sep)
{
    return join(std::ranges::begin(parts), std::ranges::end(parts), sep);
}

template <std::ranges::input_range R>
[[nodiscard]] std::string join_copy(const R& parts, std::string_view sep)
{
    return join(parts, sep);
}

template <std::ranges::input_range R>
[[nodiscard]] std::wstring join_copy(const R& parts, std::wstring_view sep)
{
    return join(parts, sep);
}

}  // namespace mt
