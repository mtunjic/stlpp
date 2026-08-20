# STL++

Not a second Standard Library. The generic machine behind one.

```mermaid
flowchart LR
  F["Functors"] -->|"plug into"| A["Algorithms"]
  A -->|"walk"| I["Iterators"]
  I -->|"sit on"| L["Layout"]
```

Functors plug into algorithms. Algorithms walk iterators. Iterators sit on layout. That is the whole design — Stepanov’s stack, in C++20, header-only, `namespace mt`. You keep `<vector>` and `<algorithm>`. You gain counted `_n` algorithms, lazy views, and containers that were never in `std`: tape, rope, zipper, ring buffer, disjoint set.

```cpp
#include "stlpp.h"

std::vector<int> v{1, 2, 3, 4};
auto prefix = mt::scan(v, mt::plus<int>{});   // {1, 3, 6, 10}
int total   = mt::fold(v, 0, mt::plus<int>{}); // 10
```

Include `"stlpp.h"` (or `"algorithms.h"`). CMake target `stlpp::stlpp`. Conan and vcpkg recipes ship the same headers. Usage examples: [user guide](doc/user-guide.md). The idea: [for beginners](doc/for-beginners.md), [why begin / end](doc/begin-end.md).

**Algorithms** — `scan`, `power`, `fold`, `gather`, `slide`, `find_if_n`, `binary_counter`  
**Hybrid** — `map`, `reduce`, `psort`, `pfind`  
**Containers** — `tape`, `rope`, `zipper`, `ring_buffer`, `disjoint_set`  
**Iterators** — `numeric_iterator`, `filter`, `windows`  
**String** — `trim`, `sanitize`

The rest: [plan / progress](doc/progress.md).

**v0.1.0** freezes this surface: `namespace mt`, include `"stlpp.h"`. Checked names keep their signatures. CUDA `scan` / `power` / `map` / `reduce` and dual-GPU `partial_bucketsort` are not in this freeze.

Recommended reading: Alexander Stepanov and Paul McJones, [*Elements of Programming*](https://elementsofprogramming.com/eop.pdf) (free PDF).

### Build

CMake 3.20+, C++20 compiler. GoogleTest and Google Benchmark are fetched automatically.

```sh
cmake -S . -B build
cmake --build build --target stlpp_tests stlpp_bench
ctest --test-dir build --output-on-failure
./build/stlpp_bench --benchmark_min_time=0.1s
python scripts/bench_history.py --min-time 0.1s
```

Or with presets: `cmake --preset sanitize && cmake --build --preset sanitize && ctest --preset sanitize`.

Dated JSON + SVG land in `bench/` (`stlpp-bench-YYYY-MM-DD`); `bench/stlpp-bench-history.svg` overlays every run. See `bench.md`.

Optional: `-DSTLPP_SANITIZE=ON` for ASan/UBSan (needs `libasan`/`libubsan`; off by default on this MinGW). Packaging (`cmake --install`, Conan, vcpkg) uses `-DSTLPP_BUILD_TESTING=OFF -DSTLPP_BUILD_BENCHMARKS=OFF` so GoogleTest/Benchmark are not fetched.

**Generic / concept tests** (EoP-style): `test/concept_axioms_test.cpp`, `test/generic_*_test.cpp`, and helpers in `test/support/`. See [`doc/progress.md`](doc/progress.md#generic-law-tests-eop-style). Behavioral reference: [Elements of Programming](https://elementsofprogramming.com/eop.pdf) and [`doc/eop-code/tests.h`](doc/eop-code/tests.h).

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

Headers land in `<prefix>/include/stlpp/` (so they do not collide with other `find.h` files). `stlpp::stlpp` adds that path; `#include "stlpp.h"` still works. There is also `<prefix>/include/stlpp.h` that forwards to the umbrella.

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

---

<img src="doc/stepanopv.jpg" alt="Alexander Stepanov" width="140">

For Alexander Stepanov.
