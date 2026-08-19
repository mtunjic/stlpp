# STL++ user guide

Header-only C++20. Include `"stlpp.h"` (or `"algorithms.h"`). Everything lives in `namespace mt`.

```cpp
#include "stlpp.h"

std::vector<int> v{1, 2, 3, 4};
auto prefix = mt::scan(v, mt::plus<int>{});   // {1, 3, 6, 10}
int total   = mt::fold(v, 0, mt::plus<int>{}); // 10
```

Functors plug into algorithms. Algorithms walk iterators. Iterators sit on layout. Most algorithms have a counted `_n` overload and a range overload.

Examples below are taken from the GoogleTest suite and the `Example:` comments in the headers.

## Setup

```cmake
target_link_libraries(your_app PRIVATE stlpp::stlpp)  # C++20
```

Submodule, `find_package(stlpp)`, Conan, and vcpkg are in the main README. Parallel algorithms need threads (`Threads::Threads` is already on the CMake target).

## Integers and iterator ops

```cpp
mt::odd(3);                            // true
mt::half(7);                           // 3  (integer divide)
mt::high_byte(std::uint16_t{0x1234});  // 0x12

std::vector<int> v{10, 20, 30, 40};
auto it = v.begin();
mt::advance(it, 2);                    // *it == 30
*mt::successor(v.begin());             // 20
*mt::predecessor(v.end());             // 40
```

## Apply, find, remove, reverse, copy

```cpp
std::vector<int> v{1, 2, 3};
mt::apply(v, [](int x) { return x * x; });          // {1, 4, 9}

std::vector<int> prefix{1, 2, 3, 4};
auto stop = mt::apply_until(prefix, [](int x) { return x + 10; },
                            [](int x) { return x == 3; });
// prefix == {11, 12, 3, 4}, *stop == 3

std::vector<int> nums{1, 3, 4, 6};
auto [it, left] = mt::find_if_n(nums.begin(), 4, [](int x) { return x % 2 == 0; });
// *it == 4, left == 2

std::vector<int> back{5, 3, 0, 1, 0, 8};
mt::find_backward(back.begin(), back.end(), 0);     // iterator to last 0
mt::find_backward(back.begin(), back.end(), 9);     // end()  (not begin())

std::vector<int> hits{1, 2, 1, 3};
auto all1 = mt::find_all(hits, 1);                  // two iterators

std::vector<int> keep{9, 0, 1, 4, 7};
keep.erase(mt::remove_if_not(keep, [](int x) { return x < 5; }), keep.end());
// {0, 1, 4}

std::list<int> xs{1, 2, 3, 4};
mt::reverse_n(xs.begin(), 4);                       // {4, 3, 2, 1}

std::vector<int> src{1, 2, 3, 4}, dst(2, 0);
auto [in, out] = mt::copy_n(src.begin(), 2, dst.begin());
// dst == {1, 2}, *in == 3
```

`find_backward*` not-found is `end()`, matching the STL.

## Slide, gather, partition, sort

```cpp
std::vector<int> w{0, 1, 2, 3, 4, 5};
auto [lo, hi] = mt::slide(w.begin() + 1, w.begin() + 3, w.begin() + 5);
// [lo, hi) still holds {1, 2}, moved as a window

std::array<int, 10> a{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
mt::gather(a.begin(), a.end(), a.begin() + 4, [](int x) { return x < 2; });
// matches of < 2 clustered around index 4

std::vector<int> part{1, 2, 3, 8, 9};
auto p = mt::partition_point_n(part.begin(), 5, [](int x) { return x < 5; });
// *p == 8  (first false, same as std::partition_point)

int mask_a[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};
bool mask[]  = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0};
auto mid = mt::stable_partition_position(std::begin(mask_a), std::end(mask_a),
    [&](int* it) { return mask[it - std::begin(mask_a)]; });
// 2 4 4 2 | 1 3 5 5 3 1

std::vector<int> s{3, 1, 4, 1};
mt::binary_insertion_sort_n(s.begin(), 4);          // {1, 1, 3, 4}
mt::sort_sums(std::vector{1, 3}, std::vector{0, 10}); // {1, 3, 11, 13}
```

## Ranges: slice, skip, take, zip, text

```cpp
std::vector<int> v{0, 1, 2, 3, 4};
auto window = mt::slice(v, 1, 3);                   // {1, 2, 3}
auto dropped = mt::skip(v, 2);                      // {2, 3, 4}
auto first3  = mt::take(v, 3);                      // {0, 1, 2}

std::vector<int> neg{-1, -2, 3, 4};
mt::skip_while(neg, [](int x) { return x < 0; });   // starts at 3
mt::take_while(neg, [](int x) { return x < 0; });   // {-1, -2}

auto pairs = mt::zip(std::vector{1, 2}, std::vector{10, 20});
// {(1,10), (2,20)}
std::vector<int> left{1, 2}, right{10, 20}, summed;
mt::zip_with(left.begin(), left.end(), right.begin(), right.end(),
             std::back_inserter(summed), std::plus<int>{});
// summed == {11, 22}
mt::pairwise(std::vector{1, 2, 3, 4});              // {(1,2),(2,3),(3,4)}
mt::concat(std::vector{1, 2}, std::vector{3});       // {1, 2, 3}
mt::group(std::vector{1, 1, 2, 2, 2, 1});           // {{1,1},{2,2,2},{1}}
mt::transpose(std::vector<std::vector<int>>{{1, 2, 3}, {4, 5, 6}});
// {{1,4},{2,5},{3,6}}
mt::border(std::vector{1, 2, 3}, 0);                // {0, 1, 2, 3, 0}

mt::shift_by(v, 2);   // rotate left
mt::shift_by(v, -2);  // rotate right

mt::lines("a\nb\n");                                // {"a","b",""}
mt::words("  one  two three ");                     // {"one","two","three"}
mt::starts_with(std::string{"https://ex.com"}, std::string{"https://"});
mt::ends_with(std::string{"file.cpp"}, std::string{".cpp"});  // end_with alias
mt::matches("ababcababa", "aba");                   // offsets {0, 5, 7}
```

```cpp
std::string line{"man,meal,moon;fat,food,feel;cat,coat,cook;love,leg,lunch"};
std::vector<std::string> groups;
mt::split(line, ";", groups);
mt::join(groups, "|");   // "man,meal,moon|fat,food,feel|..."
```

## Random and iota

Uniform closed interval `[lo, hi]`. Swapped bounds are fine.

```cpp
std::vector<double> rd(8);
mt::random(rd, 0.5, 5.0);

std::array<int, 52> deck{};
mt::random_iota(deck);                 // shuffled 0..51

std::vector<int> seq(5);
mt::iota(seq, 10);                     // {10,11,12,13,14}
mt::hill(seq.begin(), seq.end());      // {0,1,2,1,0}
```

## GCD, min/max, combinations

```cpp
mt::stein_gcd(48, 18);                 // 6
auto [g, x, y] = mt::extended_gcd(30, 12);  // 30*x + 12*y == 6

std::vector<int> mm{3, 1, 4, 1, 5};
auto [lo, hi] = mt::min_max(mm.begin(), mm.end());  // 1 and 5

std::vector<int> comb{1, 2, 3, 4};
mt::next_combination(comb.begin(), comb.begin() + 2, comb.end());
// first 2-combination becomes {1, 3} | {2, 4}
```

## EoP / list extras

`power` is a semigroup algorithm: no iterator, no container.

```cpp
mt::power(2, 10, std::multiplies<int>{});           // 1024
mt::power(3, 5, std::plus<int>{});                  // 15
mt::power(2, 0, std::multiplies<int>{}, 1);         // identity 1

mt::scan(std::vector{1, 2, 3, 4}, std::plus<>{});   // {1, 3, 6, 10}
mt::exclusive_scan(std::vector{1, 2, 3}, 0, std::plus<>{});  // {0, 1, 3}

auto step = [](int n) -> std::optional<std::pair<int, int>> {
    if (n <= 0) return std::nullopt;
    return std::pair{n, n - 1};
};
mt::unfold(step, 3);                                // {3, 2, 1}
mt::iterate_n(5, 1, [](int x) { return x * 2; });   // {1, 2, 4, 8, 16}

mt::mapcat(std::vector{1, 2}, [](int x) { return std::vector{x, x}; });
// {1, 1, 2, 2}
mt::flatten(std::vector<std::vector<int>>{{1, 2}, {}, {3}});  // {1, 2, 3}

mt::all(v, [](int x) { return x % 2 == 0; });
mt::any(v, [](int x) { return x == 4; });
mt::none(v, [](int x) { return x < 0; });
mt::count_if(v, [](int x) { return x > 2; });

mt::merge(std::vector{1, 3}, std::vector{2, 4});           // {1, 2, 3, 4}
mt::set_union(std::vector{1, 2}, std::vector{2, 3});       // {1, 2, 3}
mt::set_intersection(std::vector{1, 2, 3}, std::vector{2, 3, 4});  // {2, 3}

auto [sum, prefixes] = mt::map_accum(std::vector{1, 2, 3}, 0,
    [](int acc, int x) { return std::pair{acc + x, acc + x}; });
// sum == 6, prefixes == {1, 3, 6}

mt::inner_product(std::vector{1, 2, 3}, std::vector{4, 5, 6});  // 32
mt::adjacent_difference(std::vector{1, 3, 6, 10});             // {1, 2, 3, 4}
auto [evens, odds] = mt::partition_copy(std::vector{1, 2, 3, 4},
                                       [](int x) { return x % 2 == 0; });
mt::cartesian_product(std::vector{1, 2}, std::vector{'a', 'b'});
```

## Hybrid (CPU threads)

Sequential below 4096 elements (`mt::par::default_cutoff`). `reduce` / `psort` / `pfind` / `map` use `std::thread` above that. `partial_bucketsort` is CPU buckets; dual-GPU is not wired.

```cpp
auto squares = mt::map(std::vector{1, 2, 3}, [](int x) { return x * x; });  // {1, 4, 9}
mt::fold(squares, 0, std::plus<>{});    // 14  (always sequential left fold)
mt::reduce(squares, 0, std::plus<>{});  // 14  (parallel above cutoff)
mt::pfind(squares, 9);                  // leftmost 9
mt::psort(squares);

std::vector<int> b{9, 1, 8, 2, 7, 3, 0};
mt::partial_bucketsort(b);              // {0, 1, 2, 3, 7, 8, 9}
```

## Functors

Plug these into algorithms and into lazy iterators.

```cpp
mt::plus<int>{}(2, 3);                  // 5
mt::multiplies<int>{}(2, 3);            // 6
mt::divides<int>{}(8, 0);               // 8  (x when y == 0)
mt::minimum<int>{}(3, 1);               // 1
mt::less_than<int>{5}(3);               // true
mt::vowel{}('e');                       // true
mt::digit{}('3');                       // true
mt::is_odd(3);
mt::modulo_multiply<int> op{5};
op(3, 4);                              // 2
mt::identity_element(op);               // 1

int n = 10;
mt::halve_non_negative(n);              // 5
```

## Lazy iterators

These adapt `begin`/`end`. Algorithms can walk them the same as a vector.

```cpp
std::vector<int> v{1, 2, 3, 4, 5};

for (int x : mt::transform(v, [](int n) { return n * n; })) {}   // 1 4 9 16 25
for (int x : mt::filter(v, [](int n) { return n % 2 == 0; })) {} // 2 4
for (auto [x, y] : mt::zip_view(a, b)) {}   // stops at the shorter range
for (int x : mt::flattened(nested)) {}      // skips empty inners
for (auto w : mt::windows(v, 2)) {}         // {1,2}, {2,3}, ...
for (auto c : mt::chunks(v, 2)) {}          // {1,2}, {3,4}, {5}
for (int x : mt::cycle(v, 3)) {}            // repeat the range 3 times

std::copy(mt::numeric_iterator<int>{0}, mt::numeric_iterator<int>{4},
          std::back_inserter(out));         // 0 1 2 3
for (int x : mt::step(v, 2)) {}             // stride
for (auto [from, to] : mt::edges(v)) {}     // consecutive pairs

mt::counting_output_iterator counted;
std::copy(v.begin(), v.end(), counted);
counted.count();                            // 5
```

## Containers

**tape** — Turing tape: `deque` + head. `left()` / `right()` off an end **grows** a default cell. Also has `begin`/`end`.

```cpp
mt::tape<int> t{1, 2, 3};
t.right();
t.write(9);                                 // {1, 9, 3}
t.left();
t.read();                                   // 1
```

**ring_buffer** — fixed capacity, wrap. `try_push` fails when full (does not overwrite). Not on the iterator bus.

```cpp
mt::ring_buffer<int> r(2);
r.try_push(1);
r.try_push(2);
r.try_push(3);                              // false
r.try_pop();                                // 1
```

**zipper** — Huet zipper: focus plus O(1) local move/insert/delete. No wrap.

```cpp
mt::zipper<int> z{std::vector{1, 2, 3}};
z.focus();                                  // 1
z.move_right();
z.insert_right(9);                          // {1, 2, 9, 3}
```

**rope**, **array_slice**, **htable**, **grid**, **graph**, **dynamic_pqueue**, **quadtree**, **disjoint_set**:

```cpp
mt::rope left{"hello "};
auto phrase = mt::concat(left, mt::rope{"world"});
phrase.to_string();                         // "hello world"

std::vector<int> cells{0, 1, 2, 3, 4};
mt::array_slice<int> sl{cells.data() + 1, 3};  // {1, 2, 3}

mt::htable<int, std::string> tab;
tab.insert(3, "three");
*tab.find(3);                               // "three"

mt::vector_2d<int> m(2, 3, 0);
m(1, 2) = 7;

mt::graph g(3);
g.add_edge(0, 1);

mt::dynamic_pqueue<int, std::greater<int>> pq;  // min-heap
auto h = pq.push(5);
pq.update(h, 1);
pq.top();                                   // 1

mt::quadtree q({0, 0, 100, 100});
q.insert({10, 10});
q.query({0, 0, 20, 20});

mt::disjoint_set ds(4);
ds.unite(0, 1);
ds.connected(0, 1);                         // true
```

Thread-safe adaptors: `ts_stack`, `ts_queue`, `ts_priority_queue` — `push` / `try_pop`.

## Vector and string ext

```cpp
std::vector<int> v{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
mt::keep_if(v, mt::less_than<int>{8});
mt::distinct(v);
mt::fast_erase(v, 0);
mt::delete_if(v, [](int x) { return x % 2 == 0; });

std::string s = "woow!!!!";
mt::trim_right_if(s, '!');                  // "woow"
mt::to_lower("MARKO");                      // "marko"
mt::has_prefix(uri, "https://");
mt::sanitize(bad);
mt::is_palindrome("abba");
mt::repeat("ab", 3);                        // "ababab"
```

## Utils

```cpp
mt::print(std::vector<int>{1, 2, 3});       // [ 1, 2, 3 ]
mt::digit_sum(123);                         // 6
mt::pick_a_number(5, 15);
auto m = mt::to_map(std::vector{std::pair{1, "a"}, std::pair{2, "b"}});

mt::stopwatch t;
t.start();
/* work */
double ns = t.stop();
```

## Binary counter

Stepanov’s binary counter: occupied slots combine under a binary op.

```cpp
mt::binary_counter<int, std::plus<int>> counter(std::plus<int>{}, 0);
for (int x : values) {
    counter.add(x);
}
int sum = counter.reduce();
```

## Notes

- Prefer `"stlpp.h"`. `"algorithms.h"` only forwards to it.
- Counted `_n` APIs take a count, not a sentinel.
- `tape` grows; `ring_buffer` wraps and refuses when full. Do not mix those models.
- `reduce` / `map` / `psort` / `pfind` are sequential on short ranges; do not assume threads until you are above the cutoff.
- CUDA `scan` / `power` / `map` / `reduce` and dual-GPU `partial_bucketsort` are not in this tree.

Source of snippets: `test/*_test.cpp` and `Example:` blocks in the headers. Run `ctest --test-dir build --output-on-failure` after changing usage.
