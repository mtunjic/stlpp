# STL++ examples

Interactive runner that lists demos; you pick one by number or key.

## Build inside the monorepo

```sh
cmake -S . -B build -DSTLPP_BUILD_EXAMPLES=ON -DSTLPP_BUILD_TESTING=OFF -DSTLPP_BUILD_BENCHMARKS=OFF
cmake --build build --target stlpp_examples
./build/examples/stlpp_examples
```

Non-interactive: `./build/examples/stlpp_examples orbit`

## Build as a Conan consumer (skeleton)

From the repo root, package (or editable) `stlpp`, then:

```sh
cd examples
conan install . -of build --build=missing -s compiler.cppstd=20
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/stlpp_examples
```

If `stlpp/0.1.0` is not in the cache yet:

```sh
# from repo root
conan create . --build=missing -s compiler.cppstd=20
# or: conan editable add . stlpp/0.1.0
```

## Demos

| key | What it shows |
|-----|----------------|
| `orbit` | ρ-orbit stem/cycle via `collision_point` / `connection_point` |
| `fibpower` | Fibonacci in O(log n) with `mt::power` on matrices |
| `hotspots` | KMP `matches` → `scan` → window scores → `psort` |

More ideas are tracked in the root `TODO.md` (Examples section).
