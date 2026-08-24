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

**v0.1.0** freezes this surface: `namespace mt`, include `"stlpp.h"`. Checked names keep their signatures. CUDA `scan` / `power` / `map` / `reduce` and dual-GPU `partial_bucketsort` are not in this freeze.

Recommended reading: Alexander Stepanov and Paul McJones, [*Elements of Programming*](https://elementsofprogramming.com/eop.pdf) (free PDF).

### Build

Requires CMake 3.25+ and a C++20 compiler. On Windows, the default presets use Visual Studio 18 2026 and work from ordinary PowerShell; CMake selects the compiler, linker, and Windows SDK environment.

The shortest complete Debug cycle is:

```powershell
cmake --workflow --preset debug
```

For incremental work, run the stages separately:

```powershell
cmake --preset debug
cmake --build --preset debug --target stlpp_tests
ctest --preset debug
```

Available default Windows configurations:

| Preset | Purpose |
| --- | --- |
| `debug` | Debug build and tests |
| `release` | Optimized build and tests |
| `relwithdebinfo` | Optimized build with debug information and tests |
| `sanitize` | RelWithDebInfo plus MSVC AddressSanitizer |
| `benchmark` | Release benchmarks with IPO/LTO and no test dependency |
| `pkg` | Release package without test or benchmark dependencies |

The first four also support `cmake --workflow --preset <name>`.

Portable single-config Ninja alternatives use the `ninja-` prefix, for example:

```sh
cmake --workflow --preset ninja-debug
cmake --workflow --preset ninja-release
```

On Linux or macOS, those presets use the compiler available in the shell. With MSVC on Windows, run Ninja presets from a Developer PowerShell so `INCLUDE`, `LIB`, and the Windows SDK are initialized. The Visual Studio presets do not require that step.

Build and run benchmarks on Windows:

```powershell
cmake --preset benchmark
cmake --build --preset benchmark
.\out\build\vs\benchmark\Release\stlpp_bench.exe --benchmark_min_time=0.1s
python scripts/bench_history.py --min-time 0.1s
```

For Ninja, use `ninja-benchmark`; its executable is under `out/build/ninja-benchmark/`.

Dated JSON and SVG results land in `bench/` (`stlpp-bench-YYYY-MM-DD`); `bench/stlpp-bench-history.svg` overlays every run. See `bench.md`.

Enable the example runner in a Windows Debug preset build:

```powershell
cmake --preset debug -DSTLPP_BUILD_EXAMPLES=ON
cmake --build --preset debug --target stlpp_examples
.\out\build\vs\debug\examples\Debug\stlpp_examples.exe
```

See [`examples/README.md`](examples/README.md) for the Conan consumer skeleton.

Visual Studio is multi-config, so the build and test presets select `Debug`, `Release`, or `RelWithDebInfo` using their `configuration` fields. Ninja is single-config, so its presets set `CMAKE_BUILD_TYPE` at configure time.

GoogleTest and Google Benchmark are fetched only when their corresponding targets are enabled. Their archives are pinned by version and SHA-256. Set `STLPP_WARNINGS_AS_ERRORS=ON` for strict CI builds; it is off by default for compiler portability.

**Generic / concept tests** (EoP-style): `test/concept_axioms_test.cpp`, `test/generic_*_test.cpp`, and helpers in `test/support/`. Behavioral reference: [*Elements of Programming*](https://elementsofprogramming.com/eop.pdf).

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

Benchmarks stay off when stlpp is not the top-level project. Tests follow the parent project's standard `BUILD_TESTING` option; set it to `OFF` if the parent does not want any tests. No `find_package`.

#### 2. CMake install

From this repo:

```sh
cmake --preset pkg
cmake --build --preset pkg
cmake --install out/build/vs/pkg --config Release --prefix /path/to/prefix
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
