#pragma once

#include "concepts.h"
#include "random.h"
#include "starts_with.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <cwctype>
#include <functional>
#include <istream>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace mt {

inline int ascii_tolower(unsigned char c)
{
    return std::tolower(c);
}

inline int ascii_toupper(unsigned char c)
{
    return std::toupper(c);
}

[[nodiscard]] inline bool is_ascii_space(char c)
{
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

inline bool is_space(char c)
{
    return is_ascii_space(c);
}

/// Predicate: `c` is in `set`. Owns a copy so temporaries are safe.
///
/// Example:
///   mt::trim_right_if(s, mt::is_any_of(";,.'"));
[[nodiscard]] inline auto is_any_of(std::string set)
{
    return [set = std::move(set)](char c) { return set.find(c) != std::string::npos; };
}

template <Character Char, typename P>
std::basic_string<Char>& trim_left_if(std::basic_string<Char>& s, P pred)
{
    auto first = std::find_if_not(s.begin(), s.end(), std::move(pred));
    s.erase(s.begin(), first);
    return s;
}

template <Character Char, typename P>
std::basic_string<Char>& trim_right_if(std::basic_string<Char>& s, P pred)
{
    while (!s.empty() && std::invoke(pred, s.back())) {
        s.pop_back();
    }
    return s;
}

template <Character Char>
std::basic_string<Char>& trim_left_if(std::basic_string<Char>& s, Char ch)
{
    return trim_left_if(s, [ch](Char c) { return c == ch; });
}

template <Character Char>
std::basic_string<Char>& trim_right_if(std::basic_string<Char>& s, Char ch)
{
    return trim_right_if(s, [ch](Char c) { return c == ch; });
}

template <Character Char>
std::basic_string<Char>& trim_left(std::basic_string<Char>& s)
{
    if constexpr (std::same_as<Char, char>) {
        return trim_left_if(s, is_ascii_space);
    } else {
        return trim_left_if(s, [](Char c) { return std::iswspace(static_cast<wint_t>(c)) != 0; });
    }
}

template <Character Char>
std::basic_string<Char>& trim_right(std::basic_string<Char>& s)
{
    if constexpr (std::same_as<Char, char>) {
        return trim_right_if(s, is_ascii_space);
    } else {
        return trim_right_if(s, [](Char c) { return std::iswspace(static_cast<wint_t>(c)) != 0; });
    }
}

/// Trim ASCII/wide whitespace on both ends (in place).
///
/// Example:
///   std::string s{"  hi  "};
///   mt::trim(s);  // "hi"
template <Character Char>
std::basic_string<Char>& trim(std::basic_string<Char>& s)
{
    trim_right(s);
    return trim_left(s);
}

template <Character Char>
[[nodiscard]] std::basic_string<Char> strip_copy(std::basic_string<Char> s)
{
    trim(s);
    return s;
}

template <Character Char>
std::basic_string<Char>& strip(std::basic_string<Char>& s)
{
    return trim(s);
}

inline std::string& to_lower(std::string& s)
{
    for (char& c : s) {
        c = static_cast<char>(ascii_tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

inline std::string& to_upper(std::string& s)
{
    for (char& c : s) {
        c = static_cast<char>(ascii_toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

[[nodiscard]] inline std::string to_lower(std::string_view s)
{
    std::string out{s};
    to_lower(out);
    return out;
}

[[nodiscard]] inline std::string to_upper(std::string_view s)
{
    std::string out{s};
    to_upper(out);
    return out;
}

[[nodiscard]] inline std::string to_lower_copy(std::string_view s)
{
    return to_lower(s);
}
[[nodiscard]] inline std::string to_upper_copy(std::string_view s)
{
    return to_upper(s);
}

/// First character of each whitespace-separated word becomes upper-case.
///
/// Example:
///   mt::to_title(std::string{"hello world"});  // "Hello World"
inline std::string& to_title(std::string& s)
{
    bool new_word = true;
    for (char& c : s) {
        if (is_ascii_space(c)) {
            new_word = true;
        } else if (new_word) {
            c = static_cast<char>(ascii_toupper(static_cast<unsigned char>(c)));
            new_word = false;
        }
    }
    return s;
}

[[nodiscard]] inline std::string to_title(std::string_view s)
{
    std::string out{s};
    to_title(out);
    return out;
}

inline std::string& capitalize(std::string& s)
{
    if (!s.empty()) {
        s.front() = static_cast<char>(ascii_toupper(static_cast<unsigned char>(s.front())));
    }
    return s;
}

[[nodiscard]] inline std::string capitalize(std::string_view s)
{
    std::string out{s};
    capitalize(out);
    return out;
}

[[nodiscard]] inline std::size_t count(std::string_view s, char ch)
{
    return static_cast<std::size_t>(std::count(s.begin(), s.end(), ch));
}

[[nodiscard]] inline std::string repeat(std::string_view s, std::size_t n)
{
    if (n != 0 && s.size() > std::numeric_limits<std::size_t>::max() / n) {
        throw std::length_error("repeat overflow");
    }
    std::string out;
    out.reserve(s.size() * n);
    for (std::size_t i = 0; i < n; ++i) {
        out.append(s);
    }
    return out;
}

[[nodiscard]] inline std::string repeat_char(char c, std::size_t n)
{
    return std::string(n, c);
}

inline std::string& reverse(std::string& s)
{
    std::reverse(s.begin(), s.end());
    return s;
}

[[nodiscard]] inline std::string reverse_copy(std::string_view s)
{
    std::string out{s};
    reverse(out);
    return out;
}

template <typename T>
[[nodiscard]] T from_string(std::string_view s)
{
    T value{};
    std::istringstream in{std::string{s}};
    if (!(in >> value)) {
        throw std::invalid_argument("from_string");
    }
    in >> std::ws;
    if (!in.eof()) {
        throw std::invalid_argument("from_string");
    }
    return value;
}

template <typename T>
[[nodiscard]] T to(std::string_view s)
{
    return from_string<T>(s);
}

template <typename T>
[[nodiscard]] std::string to_string(const T& value)
{
    std::ostringstream out;
    out << value;
    return out.str();
}

[[nodiscard]] inline int to_int(std::string_view s)
{
    return from_string<int>(s);
}
[[nodiscard]] inline float to_single(std::string_view s)
{
    return from_string<float>(s);
}
[[nodiscard]] inline double to_double(std::string_view s)
{
    return from_string<double>(s);
}

inline std::string& remove_if(std::string& s, auto pred)
{
    s.erase(std::remove_if(s.begin(), s.end(), std::move(pred)), s.end());
    return s;
}

inline std::string& remove_if_not(std::string& s, auto pred)
{
    return remove_if(s, [&](char c) { return !std::invoke(pred, c); });
}

inline std::string& remove(std::string& s, char ch)
{
    return remove_if(s, [ch](char c) { return c == ch; });
}

template <typename P>
[[nodiscard]] std::string remove_copy_if_not(std::string_view s, P pred)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (std::invoke(pred, c)) {
            out.push_back(c);
        }
    }
    return out;
}

[[nodiscard]] inline bool has_prefix(std::string_view s, std::string_view prefix)
{
    return starts_with(s, prefix);
}

[[nodiscard]] inline bool has_suffix(std::string_view s, std::string_view suffix)
{
    return ends_with(s, suffix);
}

[[nodiscard]] inline bool has_char(std::string_view s, char ch)
{
    return s.find(ch) != std::string_view::npos;
}

[[nodiscard]] inline bool is_digit(std::string_view s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) {
        return std::isdigit(static_cast<unsigned char>(c)) != 0;
    });
}

[[nodiscard]] inline bool is_alpha(std::string_view s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) {
        return std::isalpha(static_cast<unsigned char>(c)) != 0;
    });
}

[[nodiscard]] inline bool is_palindrome(std::string_view s)
{
    return std::equal(s.begin(), s.begin() + static_cast<std::ptrdiff_t>(s.size() / 2), s.rbegin());
}

[[nodiscard]] inline bool eq_ignore_case(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (ascii_tolower(static_cast<unsigned char>(a[i])) !=
            ascii_tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

/// Compare after applying `proj` to each character. Negative / zero / positive.
template <typename F>
[[nodiscard]] int compare_by(std::string_view a, std::string_view b, F proj)
{
    const std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) {
        const auto x = std::invoke(proj, a[i]);
        const auto y = std::invoke(proj, b[i]);
        if (x < y) {
            return -1;
        }
        if (y < x) {
            return 1;
        }
    }
    if (a.size() < b.size()) {
        return -1;
    }
    if (b.size() < a.size()) {
        return 1;
    }
    return 0;
}

[[nodiscard]] inline std::vector<char> to_chars(std::string_view s)
{
    return {s.begin(), s.end()};
}

[[nodiscard]] inline std::string random_string(std::size_t n)
{
    static constexpr char alphabet[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string out(n, '\0');
    if (n == 0) {
        return out;
    }
    random(out.begin(), out.end(), 0, static_cast<int>(sizeof(alphabet) - 2), default_rng());
    for (char& c : out) {
        c = alphabet[static_cast<unsigned char>(c)];
    }
    return out;
}

/// Treat each extra character as whitespace for `operator>>` on `stream`.
///
/// Example:
///   std::istringstream in{"1,2,3"};
///   mt::setdelim(in, ",");
///   int a, b, c; in >> a >> b >> c;  // 1 2 3
inline void setdelim(std::ios& stream, std::string_view extra)
{
    using mask = std::ctype<char>::mask;
    auto table = std::make_unique<mask[]>(std::ctype<char>::table_size);
    std::copy(std::ctype<char>::classic_table(),
              std::ctype<char>::classic_table() + std::ctype<char>::table_size, table.get());
    for (unsigned char ch : extra) {
        table[ch] = static_cast<mask>(table[ch] | std::ctype_base::space);
    }
    stream.imbue(std::locale(stream.getloc(), new std::ctype<char>(table.release(), true)));
}

[[nodiscard]] inline std::string letters_copy(std::string_view s)
{
    std::string out;
    for (char c : s) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            out.push_back(c);
        }
    }
    return out;
}

[[nodiscard]] inline std::string numbers_copy(std::string_view s)
{
    std::string out;
    for (char c : s) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            out.push_back(c);
        }
    }
    return out;
}

[[nodiscard]] inline std::wstring letters_copy(std::wstring_view s)
{
    std::wstring out;
    for (wchar_t c : s) {
        if (std::iswalpha(static_cast<wint_t>(c))) {
            out.push_back(c);
        }
    }
    return out;
}

[[nodiscard]] inline std::wstring numbers_copy(std::wstring_view s)
{
    std::wstring out;
    for (wchar_t c : s) {
        if (std::iswdigit(static_cast<wint_t>(c))) {
            out.push_back(c);
        }
    }
    return out;
}

[[nodiscard]] inline std::wstring digits_copy(std::wstring_view s)
{
    return numbers_copy(s);
}

[[nodiscard]] inline std::vector<double> floats_copy(std::string_view s)
{
    std::vector<double> out;
    const char* p = s.data();
    const char* end = p + s.size();
    while (p < end) {
        double value = 0;
        const auto [next, ec] = std::from_chars(p, end, value);
        if (ec == std::errc{} && next != p) {
            out.push_back(value);
            p = next;
        } else {
            ++p;
        }
    }
    return out;
}

[[nodiscard]] inline std::size_t count_digits(std::string_view s)
{
    return numbers_copy(s).size();
}

[[nodiscard]] inline std::size_t count_letters(std::string_view s)
{
    return letters_copy(s).size();
}

/// Keep letters and digits, then lower-case.
///
/// Example:
///   mt::sanitize(std::string{"Hello, 12!"});  // "hello12"
inline std::string& sanitize(std::string& s)
{
    remove_if(s, [](char c) { return std::isalnum(static_cast<unsigned char>(c)) == 0; });
    return to_lower(s);
}

[[nodiscard]] inline std::string sanitize_copy(std::string_view s)
{
    std::string out{s};
    sanitize(out);
    return out;
}

inline std::wstring& sanitize(std::wstring& s)
{
    s.erase(std::remove_if(s.begin(), s.end(),
                           [](wchar_t c) { return std::iswalnum(static_cast<wint_t>(c)) == 0; }),
            s.end());
    for (wchar_t& c : s) {
        c = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(c)));
    }
    return s;
}

/// Pad `s` on the left to `width` with `fill`. No-op if already long enough.
///
/// Example:
///   std::string s{"7"};
///   mt::left_pad(s, 3, '0');  // "007"
inline std::string& left_pad(std::string& s, std::size_t width, char fill = ' ')
{
    if (s.size() < width) {
        s.insert(s.begin(), width - s.size(), fill);
    }
    return s;
}

inline std::string& right_pad(std::string& s, std::size_t width, char fill = ' ')
{
    if (s.size() < width) {
        s.append(width - s.size(), fill);
    }
    return s;
}

inline std::string& center(std::string& s, std::size_t width, char fill = ' ')
{
    if (s.size() >= width) {
        return s;
    }
    const std::size_t add = width - s.size();
    const std::size_t left = add / 2;
    s.insert(s.begin(), left, fill);
    s.append(add - left, fill);
    return s;
}

/// Remove the last character (Ruby `chop`). Empty string is unchanged.
///
/// Example:
///   std::string s{"hi!"};
///   mt::chop(s);  // "hi"
inline std::string& chop(std::string& s)
{
    if (!s.empty()) {
        s.pop_back();
    }
    return s;
}

/// Remove a trailing `\n`, `\r`, or `\r\n` (Ruby `chomp`).
inline std::string& chomp(std::string& s)
{
    if (s.size() >= 2 && s.ends_with("\r\n")) {
        s.resize(s.size() - 2);
        return s;
    }
    if (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
    return s;
}

/// Replace the first occurrence of `from` with `to`. Empty `from` is a no-op.
///
/// Example:
///   std::string s{"aa ba"};
///   mt::replace(s, "a", "x");  // "xa ba"
inline std::string& replace(std::string& s, std::string_view from, std::string_view to)
{
    if (from.empty()) {
        return s;
    }
    if (const auto p = s.find(from); p != std::string::npos) {
        s.replace(p, from.size(), to);
    }
    return s;
}

[[nodiscard]] inline std::string replace_copy(std::string s, std::string_view from,
                                              std::string_view to)
{
    replace(s, from, to);
    return s;
}

/// Replace every occurrence of `from` with `to`. Empty `from` is a no-op.
///
/// Example:
///   std::string s{"aa ba"};
///   mt::gsub(s, "a", "x");  // "xx bx"
inline std::string& gsub(std::string& s, std::string_view from, std::string_view to)
{
    if (from.empty()) {
        return s;
    }
    std::size_t p = 0;
    while ((p = s.find(from, p)) != std::string::npos) {
        s.replace(p, from.size(), to);
        p += to.size();
    }
    return s;
}

[[nodiscard]] inline std::string gsub_copy(std::string s, std::string_view from,
                                           std::string_view to)
{
    gsub(s, from, to);
    return s;
}

[[nodiscard]] inline bool contains(std::string_view s, std::string_view needle)
{
    return s.find(needle) != std::string_view::npos;
}

[[nodiscard]] inline bool contains(std::string_view s, char ch)
{
    return s.find(ch) != std::string_view::npos;
}

[[nodiscard]] inline bool contains_any(std::string_view s, std::string_view set)
{
    return s.find_first_of(set) != std::string_view::npos;
}

/// Naive English plural: `y` → `ies`, sibilants take `es`, otherwise `s`.
///
/// Example:
///   mt::pluralize("city");   // "cities"
///   mt::pluralize("box");    // "boxes"
[[nodiscard]] inline std::string pluralize(std::string_view word)
{
    std::string s{word};
    if (s.empty()) {
        return s;
    }
    const auto lower_at = [](std::string_view w, std::size_t i) {
        return static_cast<char>(ascii_tolower(static_cast<unsigned char>(w[i])));
    };
    const char last = lower_at(s, s.size() - 1);
    if (s.size() >= 2 && last == 'y') {
        const char prev = lower_at(s, s.size() - 2);
        if (prev != 'a' && prev != 'e' && prev != 'i' && prev != 'o' && prev != 'u') {
            s.pop_back();
            s += "ies";
            return s;
        }
    }
    if (last == 's' || last == 'x' || last == 'z') {
        s += "es";
        return s;
    }
    if (s.size() >= 2) {
        const char prev = lower_at(s, s.size() - 2);
        if ((prev == 'c' && last == 'h') || (prev == 's' && last == 'h')) {
            s += "es";
            return s;
        }
    }
    s.push_back('s');
    return s;
}

/// Repeating-key XOR. Applying the same key twice restores the original.
/// Not a cryptographic primitive — a string toy.
///
/// Example:
///   std::string s{"secret"};
///   mt::crypt(s, "k");
///   mt::crypt(s, "k");  // "secret"
inline std::string& crypt(std::string& s, std::string_view key)
{
    if (key.empty()) {
        return s;
    }
    for (std::size_t i = 0; i < s.size(); ++i) {
        s[i] = static_cast<char>(static_cast<unsigned char>(s[i]) ^
                                 static_cast<unsigned char>(key[i % key.size()]));
    }
    return s;
}

[[nodiscard]] inline std::string crypt_copy(std::string s, std::string_view key)
{
    crypt(s, key);
    return s;
}

}  // namespace mt
