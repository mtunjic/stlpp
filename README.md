# STL++

Not a second Standard Library. The generic machine behind one.

Functors plug into algorithms. Algorithms walk iterators. Iterators sit on layout. That is the whole design — Stepanov’s stack, in C++20, header-only, `namespace mt`. You keep `<vector>` and `<algorithm>`. You gain counted `_n` algorithms, lazy views, and containers that were never in `std`: tape, rope, zipper, ring buffer, disjoint set.

```cpp
#include "stlpp.h"

std::vector<int> v{1, 2, 3, 4};
auto prefix = mt::scan(v, mt::plus<int>{});   // {1, 3, 6, 10}
int total   = mt::fold(v, 0, mt::plus<int>{}); // 10
```

Include `"stlpp.h"` (or `"algorithms.h"`). CMake target `stlpp::stlpp`. Conan and vcpkg recipes ship the same headers. Usage examples from the tests: [user guide](doc/user-guide.md).

### Build

CMake 3.20+, C++20 compiler. GoogleTest and Google Benchmark are fetched automatically.

```sh
cmake -S . -B build
cmake --build build --target stlpp_tests stlpp_bench
ctest --test-dir build --output-on-failure
./build/stlpp_bench --benchmark_min_time=0.1s
python scripts/bench_history.py --min-time 0.1s
```

Dated JSON + SVG land in `bench/` (`stlpp-bench-YYYY-MM-DD`); `bench/stlpp-bench-history.svg` overlays every run. See `bench.md`.

Optional: `-DSTLPP_SANITIZE=ON` for ASan/UBSan (needs `libasan`/`libubsan`; off by default on this MinGW). Packaging (`cmake --install`, Conan, vcpkg) uses `-DSTLPP_BUILD_TESTING=OFF -DSTLPP_BUILD_BENCHMARKS=OFF` so GoogleTest/Benchmark are not fetched.

### Use in an existing C++ project

Header-only, C++20, `namespace mt`. Pick **one** path. Conan and vcpkg both work as recipes, but not in the same CMake configure (each owns `CMAKE_TOOLCHAIN_FILE`).

After any path:

```cmake
# CMakeLists.txt — C++20
find_package(stlpp REQUIRED)          # skip this line if you used add_subdirectory
target_link_libraries(your_app PRIVATE stlpp::stlpp)
```

```cpp
#include "stlpp.h"

std::vector<int> v{1, 2, 3, 4};
int s = mt::fold(v, 0, mt::plus<int>{});  // 10
```

#### 1. Git submodule (simplest)

```sh
git submodule add git@github.com:mtunjic/stlpp.git third_party/stlpp
```

```cmake
add_subdirectory(third_party/stlpp)
target_link_libraries(your_app PRIVATE stlpp::stlpp)
```

Tests/benches stay off when stlpp is not the top-level project. No `find_package`.

#### 2. CMake install

From this repo:

```sh
cmake -S . -B build-pkg -DSTLPP_BUILD_TESTING=OFF -DSTLPP_BUILD_BENCHMARKS=OFF
cmake --install build-pkg --prefix /path/to/prefix
```

In your app:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/prefix
```

#### 3. Conan 2

Once from this repo (puts `stlpp/0.1.0` in the local cache):

```sh
conan create . --build=missing -s compiler.cppstd=20
```

In your app, `conanfile.txt`:

```ini
[requires]
stlpp/0.1.0

[generators]
CMakeDeps
CMakeToolchain
```

```sh
conan install . --output-folder=build --build=missing -s compiler.cppstd=20
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Auto-detected MSVC profiles are often `compiler.cppstd=14`; pass `20`.

#### 4. vcpkg overlay

```sh
vcpkg install stlpp --overlay-ports=/path/to/stlpp/ports
```

Or in your app `vcpkg.json`:

```json
{ "dependencies": ["stlpp"] }
```

```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
```

Point vcpkg at this repo’s overlay with `VCPKG_OVERLAY_PORTS=/path/to/stlpp/ports` (or `--overlay-ports`) until the port is in a registry.

### Algorithms

Sequential algorithms below are C++20, documented with a short flow/example comment, covered by GoogleTest, and exercised by Google Benchmark (`test/*_bench.cpp`).

- [x] binary_counter ~3d #algo @mtunjic 2026-08-19
- [x] split, join ~1d #algo @mtunjic 2026-08-19
- [x] random ~1d #algo @mtunjic 2026-08-19
- [x] slice ~1d #algo @mtunjic 2026-08-19
- [x] random_iota ~1d #algo @mtunjic 2026-08-19
- [x] apply ~1d #algo @mtunjic 2026-08-19
- [x] apply_until ~1d #algo @mtunjic 2026-08-19
- [x] find_all ~1d #algo @mtunjic 2026-08-19
- [x] find_if_n ~1d #algo @mtunjic 2026-08-19
- [x] find_backward_if_n ~1d #algo @mtunjic 2026-08-19
- [x] find_backward ~1d #algo @mtunjic 2026-08-19
- [x] find_backward_if ~1d #algo @mtunjic 2026-08-19
- [x] find_backward_if_not ~1d #algo @mtunjic 2026-08-19
- [x] remove_if_not ~1d #algo @mtunjic 2026-08-19
- [x] remove_if_not_backward ~1d #algo @mtunjic 2026-08-19
- [x] reverse_n ~1d #algo @mtunjic 2026-08-19
- [x] reverse_until ~1d #algo @mtunjic 2026-08-19
- [x] reverse_n_with_buffer ~1d #algo @mtunjic 2026-08-19
- [x] swap_ranges_n ~1d #algo @mtunjic 2026-08-19
- [x] partition_point_n ~1d #algo @mtunjic 2026-08-19
- [x] stable_partition_position ~2d #algo @mtunjic 2026-08-19
- [x] copy_n ~1d #algo @mtunjic 2026-08-19
- [x] copy_n_with_buffer ~1d #algo @mtunjic 2026-08-19
- [x] slide ~1d #algo @mtunjic 2026-08-19
- [x] gather ~1d #algo @mtunjic 2026-08-19
- [x] min_max ~1d #algo @mtunjic 2026-08-19
- [x] stein_gcd ~1d #algo @mtunjic 2026-08-19
- [x] extended_gcd ~1d #algo @mtunjic 2026-08-19
- [x] binary_insertion_sort_n ~2d #algo @mtunjic 2026-08-19
- [x] upper_bound_n ~1d #algo @mtunjic 2026-08-19
- [x] sort_sums ~1d #algo @mtunjic 2026-08-19
- [x] sort_subrange ~2d #algo @mtunjic 2026-08-19
- [x] median_sort ~2d #algo @mtunjic 2026-08-19
- [x] zip, unzip, zip_with, zip_n ~2d #algo @mtunjic 2026-08-19
- [x] skip, skip_while ~1d #algo @mtunjic 2026-08-19
- [x] take, take_while ~1d #algo @mtunjic 2026-08-19
- [x] pairwise ~1d #algo @mtunjic 2026-08-19
- [x] concat ~1d #algo @mtunjic 2026-08-19
- [x] transpose ~1d #algo @mtunjic 2026-08-19
- [x] intercalate ~1d #algo @mtunjic 2026-08-19
- [x] group ~1d #algo @mtunjic 2026-08-19
- [x] lines, unlines ~1d #algo @mtunjic 2026-08-19
- [x] words, unwords ~1d #algo @mtunjic 2026-08-19
- [x] starts_with, ends_with ~1d #algo @mtunjic 2026-08-19
- [x] shift_by ~1d #algo @mtunjic 2026-08-19
- [x] border ~1d #algo @mtunjic 2026-08-19
- [x] matches ~2d #algo @mtunjic 2026-08-19
- [x] next_combination ~2d #algo @mtunjic 2026-08-19

### EoP / Lisp / Haskell extras

Iterator algorithms and views from EoP, Data.List, and Lisp. `scan` / `power` / `map` / `reduce` are the CUDA-shaped vocabulary later.

- [x] power, power with identity ~2d #eop @mtunjic 2026-08-19
- [x] scan, exclusive_scan ~2d #eop @mtunjic 2026-08-19
- [x] unfold, iterate_n ~2d #eop @mtunjic 2026-08-19
- [x] span_if, break_if, split_at ~1d #eop @mtunjic 2026-08-19
- [x] mapcat, flatten ~1d #eop @mtunjic 2026-08-19
- [x] all, any, none, count_if ~1d #eop @mtunjic 2026-08-19
- [x] collision_point ~2d #eop @mtunjic 2026-08-19
- [x] lower_bound_n ~1d #eop @mtunjic 2026-08-19
- [x] merge, set_union, set_intersection ~2d #eop @mtunjic 2026-08-19
- [x] map_accum, group_by, inner_product, adjacent_difference ~2d #eop @mtunjic 2026-08-19
- [x] partition_copy, cartesian_product, next_permutation ~2d #eop @mtunjic 2026-08-19

### Hybrid, parallel and adaptive

CPU threads (`std::thread`), sequential below 4096 elements. `partial_bucketsort` is a parallel CPU bucket sort — dual-GPU offload is not wired (no CUDA/HIP in this tree).

- [x] psort ~3d #hybrid @mtunjic 2026-08-19
- [x] partial_bucketsort ~3d #hybrid @mtunjic 2026-08-19
- [x] map ~2d #hybrid @mtunjic 2026-08-19
- [x] reduce ~2d #hybrid @mtunjic 2026-08-19
- [x] fold ~1d #hybrid @mtunjic 2026-08-19
- [x] pfind ~2d #hybrid @mtunjic 2026-08-19
- [ ] dual-GPU partial_bucketsort ~5d #cuda @mtunjic 2026-08-19
- [ ] CUDA scan, power, map, reduce ~8d #cuda @mtunjic 2026-08-19

### Containers

C++20 containers in `containers/`. The Slice container is `array_slice` — distinct from the `slice` algorithm.

- [x] tape ~2d #container @mtunjic 2026-08-19
- [x] rope ~3d #container @mtunjic 2026-08-19
- [x] array_slice ~1d #container @mtunjic 2026-08-19
- [x] htable ~3d #container @mtunjic 2026-08-19
- [x] grid ~1d #container @mtunjic 2026-08-19
- [x] vector_2d, vector_3d ~1d #container @mtunjic 2026-08-19
- [x] graph ~2d #container @mtunjic 2026-08-19
- [x] dynamic_pqueue ~2d #container @mtunjic 2026-08-19
- [x] quadtree ~3d #container @mtunjic 2026-08-19
- [x] zipper ~2d #container @mtunjic 2026-08-19
- [x] ring_buffer ~2d #container @mtunjic 2026-08-19
- [x] disjoint_set ~1d #container @mtunjic 2026-08-19

### Thread-safe Container Adaptors

- [x] ts_stack ~1d #container @mtunjic 2026-08-19
- [x] ts_queue ~1d #container @mtunjic 2026-08-19
- [x] ts_priority_queue ~1d #container @mtunjic 2026-08-19

### Iterator Adaptors

C++20 iterators in `iterators/`. `numeric_iterator` counts; `step_iterator` strides; `edge_iterator` walks consecutive pairs. Transform/filter/zip/flatten/window are lazy Data.List views.

- [x] numeric_iterator ~1d #iterator @mtunjic 2026-08-19
- [x] step_iterator ~1d #iterator @mtunjic 2026-08-19
- [x] edge_iterator ~1d #iterator @mtunjic 2026-08-19
- [x] transform_iterator ~2d #iterator @mtunjic 2026-08-19
- [x] filter_iterator ~2d #iterator @mtunjic 2026-08-19
- [x] zip_iterator ~2d #iterator @mtunjic 2026-08-19
- [x] flatten_iterator ~2d #iterator @mtunjic 2026-08-19
- [x] window_iterator, chunk_iterator ~2d #iterator @mtunjic 2026-08-19
- [x] cycle_iterator ~1d #iterator @mtunjic 2026-08-19
- [x] unfold_iterator ~2d #iterator @mtunjic 2026-08-19
- [x] counting_output_iterator ~1d #iterator @mtunjic 2026-08-19

### Functors

C++20 function objects in `algorithms/functors.h`.

- [x] conversion ~1d #functor @mtunjic 2026-08-19
- [x] square_root ~1d #functor @mtunjic 2026-08-19
- [x] vowel ~1d #functor @mtunjic 2026-08-19
- [x] multiplies ~1d #functor @mtunjic 2026-08-19
- [x] divides ~1d #functor @mtunjic 2026-08-19
- [x] cube_root_pow ~1d #functor @mtunjic 2026-08-19
- [x] root ~1d #functor @mtunjic 2026-08-19
- [x] norm ~1d #functor @mtunjic 2026-08-19
- [x] digit ~1d #functor @mtunjic 2026-08-19
- [x] plus ~1d #functor @mtunjic 2026-08-19
- [x] minimum ~1d #functor @mtunjic 2026-08-19
- [x] less_than, greater_than ~1d #functor @mtunjic 2026-08-19
- [x] to_lower, to_upper, non_letter ~1d #functor @mtunjic 2026-08-19
- [x] is_zero ~1d #functor @mtunjic 2026-08-19
- [x] is_positive, halve_non_negative ~1d #functor @mtunjic 2026-08-19
- [x] is_odd, is_even ~1d #functor @mtunjic 2026-08-19
- [x] modulo_multiply ~1d #functor @mtunjic 2026-08-19
- [x] identity_element ~1d #functor @mtunjic 2026-08-19
- [x] printer ~1d #functor @mtunjic 2026-08-19

### Vector ext

`extensions/vector_ext.h` — `keep_if` / `delete_if` / `distinct` / `fast_erase` / `read`.

- [x] keep_if, keep_if_not ~1d #vector @mtunjic 2026-08-19
- [x] delete_if, delete_if_not ~1d #vector @mtunjic 2026-08-19
- [x] distinct ~1d #vector @mtunjic 2026-08-19
- [x] fast_erase ~1d #vector @mtunjic 2026-08-19
- [x] read, read_nums ~1d #vector @mtunjic 2026-08-19

### String ext

`extensions/string_ext.h`.

- [x] trim ~1d #string @mtunjic 2026-08-19
- [x] trim_left, trim_right ~1d #string @mtunjic 2026-08-19
- [x] trim_right_if, trim_left_if ~1d #string @mtunjic 2026-08-19
- [x] left_pad, right_pad, center ~1d #string @mtunjic 2026-08-19
- [x] to_lower, to_title, to_upper ~1d #string @mtunjic 2026-08-19
- [x] to_upper_copy, to_lower_copy ~1d #string @mtunjic 2026-08-19
- [x] chop, chomp ~1d #string @mtunjic 2026-08-19
- [x] count, repeat ~1d #string @mtunjic 2026-08-19
- [x] replace, replace_copy ~1d #string @mtunjic 2026-08-19
- [x] reverse, reverse_copy ~1d #string @mtunjic 2026-08-19
- [x] gsub, gsub_copy ~1d #string @mtunjic 2026-08-19
- [x] contains, contains_any ~1d #string @mtunjic 2026-08-19
- [x] to, from_string, to_string, to_int, to_single, to_double ~2d #string @mtunjic 2026-08-19
- [x] remove, remove_if, remove_if_not, remove_copy_if_not ~1d #string @mtunjic 2026-08-19
- [x] has_prefix, has_suffix ~1d #string @mtunjic 2026-08-19
- [x] is_digit, is_palindrome, is_alpha ~1d #string @mtunjic 2026-08-19
- [x] compare_by, eq_ignore_case ~1d #string @mtunjic 2026-08-19
- [x] pluralize ~1d #string @mtunjic 2026-08-19
- [x] split, split_copy ~1d #string @mtunjic 2026-08-19
- [x] join, join_copy ~1d #string @mtunjic 2026-08-19
- [x] has_char, repeat_char, to_chars ~1d #string @mtunjic 2026-08-19
- [x] random_string ~1d #string @mtunjic 2026-08-19
- [x] setdelim ~1d #string @mtunjic 2026-08-19
- [x] letters_copy, numbers_copy, floats_copy ~1d #string @mtunjic 2026-08-19
- [x] count_digits, count_letters ~1d #string @mtunjic 2026-08-19
- [x] sanitize, sanitize_copy, capitalize ~1d #string @mtunjic 2026-08-19
- [x] strip, strip_copy ~1d #string @mtunjic 2026-08-19
- [x] crypt ~1d #string @mtunjic 2026-08-19

### Utils

- [x] to_map, to_list, to_array ~1d #util @mtunjic 2026-08-19
- [x] stopwatch ~1d #util @mtunjic 2026-08-19
- [x] thread_guard, join_threads ~1d #util @mtunjic 2026-08-19
- [x] print ~1d #util @mtunjic 2026-08-19
- [x] digit_sum ~1d #util @mtunjic 2026-08-19
- [x] randomize, pick_a_number ~1d #util @mtunjic 2026-08-19
- [x] wait ~1d #util @mtunjic 2026-08-19
- [x] bench ~1d #util @mtunjic 2026-08-19


``` cpp
#include "stlpp.h"

// apply / apply_until
std::vector<int> squares{1, 2, 3};
mt::apply(squares, [](int x) { return x * x; });          // {1, 4, 9}

std::vector<int> prefix{1, 2, 3, 4};
mt::apply_until(prefix, [](int x) { return x + 10; },
                [](int x) { return x == 3; });            // {11, 12, 3, 4}

// find_if_n — at most n steps; returns {iterator, remaining}
std::vector<int> v{1, 3, 4, 6};
auto [it, left] = mt::find_if_n(v.begin(), 4, [](int x) { return x % 2 == 0; });
// *it == 4, left == 2

// find_backward / find_backward_if — not-found is end(), not begin()
auto p = mt::find_backward_if(v.begin(), v.end(),
                              [](int x) { return x == 0; });
if (p != v.end()) {
    // found
}

// gather — elements matching pred clustered around pos
std::array<int, 10> a10{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
auto insertPos = std::begin(a10) + 4;
mt::gather(std::begin(a10), std::end(a10), insertPos,
           [](int x) { return x < 2; });
/*
    Before: [ 9, 0, 1, 4, 4, 0, 7, 7, 8, 0 ]
    After gathering all < 2 around index 4:
    the run [first_kept, last_kept) holds 0, 1, 0, 0
*/

// remove_if_not — keep elements that satisfy pred
std::vector<int> v1{9, 0, 1, 4, 4, 0, 7, 7, 8, 0};
auto tail = mt::remove_if_not(v1, [](int x) { return x < 5; });
v1.erase(tail, v1.end());
// [ 0, 1, 4, 4, 0, 0 ]

// stable_partition_position — predicate on the iterator, not the value
int a[] = {1, 2, 3, 4, 5, 5, 4, 3, 2, 1};
bool mask[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0};
auto mid = mt::stable_partition_position(std::begin(a), std::end(a),
    [&](int* it) { return mask[it - std::begin(a)]; });
// 2 4 4 2 ^ 1 3 5 5 3 1

// random — uniform closed interval [lo, hi] (swapped bounds are fine)
std::vector<double> rd(8);
mt::random(rd, 0.5, 5.0);

std::array<int, 52> deck{};
mt::random_iota(deck);   // shuffled 0..51

// split / join
std::string line{"man,meal,moon;fat,food,feel;cat,coat,cook;love,leg,lunch"};
std::vector<std::string> groups;
mt::split(line, ";", groups);
mt::join(groups, "|");   // "man,meal,moon|fat,food,feel|..."

// slice, skip, take
std::vector<int> nums{0, 1, 2, 3, 4};
auto window = mt::slice(nums, 1, 3);          // {1, 2, 3}
auto dropped = mt::skip(nums, 2);             // {2, 3, 4}
auto first3 = mt::take(nums, 3);              // {0, 1, 2}

// zip / unzip
auto pairs = mt::zip(std::vector{1, 2}, std::vector{10, 20});
// {(1,10), (2,20)}

// gcd
mt::stein_gcd(48, 18);                        // 6
auto [g, x, y] = mt::extended_gcd(30, 12);    // 30*x + 12*y == 6

// starts_with / ends_with (end_with is an alias)
mt::starts_with(std::string{"https://ex.com"}, std::string{"https://"});
mt::ends_with(std::string{"file.cpp"}, std::string{".cpp"});

// KMP
mt::matches("ababcababa", "aba");             // offsets {0, 5, 7}

// next combination of k=2 from {1,2,3,4}
std::vector<int> comb{1, 2, 3, 4};
mt::next_combination(comb.begin(), comb.begin() + 2, comb.end());
// {1, 3} | {2, 4}

// numeric / step / edge iterators
std::copy(mt::numeric_iterator<int>{0}, mt::numeric_iterator<int>{4},
          std::back_inserter(v));          // appends 0,1,2,3
for (int x : mt::step(v, 2)) {}            // every second element
for (auto [from, to] : mt::edges(v)) {}    // consecutive pairs

// tape / rope / array_slice
mt::tape<int> t{1, 2, 3};
t.right();
t.write(9);                               // {1, 9, 3}

mt::rope left{"hello "};
auto phrase = mt::concat(left, mt::rope{"world"});
phrase.to_string();                       // "hello world"

std::vector<int> cells{0, 1, 2, 3, 4};
mt::array_slice<int> window{cells.data() + 1, 3};  // {1, 2, 3}

// htable / vector_2d / graph / dynamic_pqueue
mt::htable<int, std::string> tab;
tab.insert(3, "three");
*tab.find(3);                             // "three"

mt::vector_2d<int> m(2, 3, 0);
m(1, 2) = 7;

mt::graph g(3);
g.add_edge(0, 1);

mt::dynamic_pqueue<int, std::greater<int>> pq;  // min-heap
auto h = pq.push(5);
pq.update(h, 1);
pq.top();                                 // 1

// map / fold / reduce / pfind / psort
auto squares = mt::map(std::vector{1, 2, 3}, [](int x) { return x * x; });
mt::fold(squares, 0, std::plus<>{});      // 14
mt::reduce(squares, 0, std::plus<>{});    // 14 (parallel above cutoff)
mt::pfind(squares, 9);                    // leftmost 9
mt::psort(squares);

// power / scan / mapcat / lazy views
mt::power(2, 10, std::multiplies<int>{});          // 1024
mt::scan(std::vector{1, 2, 3, 4}, std::plus<>{});  // {1, 3, 6, 10}
for (int x : mt::filter(v, [](int n) { return n % 2 == 0; })) {}
for (auto w : mt::windows(v, 2)) {}

mt::zipper<int> z{std::vector{1, 2, 3}};
z.move_right();
mt::disjoint_set ds(4);
ds.unite(0, 1);
```

#### String ext
```cpp
// trim_right_if
std::string  s  = "woow!!!!";
std::wstring ws = L"Awesome........";
std::string  sp = "lot of space       ";
trim_right_if(s, '!');   // => woow
trim_right_if(sp, ' ');  // => lot of space
trim_right_if(ws, L'.'); // => Awesome

std::string s1 {"';;;'yep..'';"};
s2 = trim_right_if(s1, is_any_of(";,.'")); // => yep
std::string srcp {"trim me     "};         // => len: 12
// same as default trim_right
trim_right_if(srcp, is_space);             // => trim me (len: 7)


std::wstring badstr {L"some,12<script>-1=1test29710-1---,.1888Osss"};
sanitize(badstr); 	  // =>  some12script11test2971011888osss
digits_copy(badstr);  // =>  12112971011888
letters_copy(badstr); // =>  somescripttestosss

// prefix suffix
std::string uristr {"https://www.google.com"};

if (has_prefix(uristr, "https://"))
	std::cout << "yes!" << std::endl;

if (has_suffix(uristr, ".com"))
	std::cout << "yes!" << std::endl;

// find_f and find_if_not
std::string sr{"-1,2,3;name=marko$"};
// remove all digits
remove_if(sr, ::isdigit);			// => -,,;name=marko$
// remove all non-digits
remove_if_not(sr2, ::isdigit);	// => 123


//find
std::string fs {"-some1+2"};
auto p = find_if_backward(fs.begin(), fs.end(),
						[](const char c) {return c == '+';});
if (p != fs.end())
	std::cout << "\nfound " << *p << "at "
	          << std::distance(begin(fs), p) << '\n';
// found + at 6

// to_lower, to_upper...
to_lower("MARKO") // => marko
to_upper("cpp")   // => CPP
```
