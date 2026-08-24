# STL++ examples

Interactive runner that lists demos; you pick one by number or key.

## Build inside the monorepo

```powershell
cmake --preset debug -DSTLPP_BUILD_EXAMPLES=ON -DBUILD_TESTING=OFF
cmake --build --preset debug --target stlpp_examples
.\out\build\vs\debug\examples\Debug\stlpp_examples.exe
```

Non-interactive: `.\out\build\vs\debug\examples\Debug\stlpp_examples.exe orbit`

## Build as a Conan consumer (skeleton)

From the repo root, package (or editable) `stlpp`, then:

```sh
cd examples
conan install . -of build --build=missing -s compiler.cppstd=20
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/stlpp_examples
```

If the matching `stlpp/<version>` is not in the cache yet:

```sh
# from repo root
conan create . --build=missing -s compiler.cppstd=20
# or: conan editable add .
```

## Demos

| key | What it shows |
|-----|----------------|
| `orbit` | ρ-orbit stem/cycle via `collision_point` / `connection_point` |
| `fibpower` | Fibonacci in O(log n) with `mt::power` on matrices |
| `hotspots` | KMP `matches` → `scan` → window scores → `psort` |
| `btree` | Expr trees: traverse/eval; isomorphic ≠ equal |
| `rope` | Rope concat vs `string +=`; zipper focus edit |
| `parallel` | Timed `fold` vs `reduce` and sequential vs parallel `map` |
| `cards` | Gather / slide / `partition_stable` on a shuffled deck |
| `components` | Grid components via `graph` + `disjoint_set` |
| `pipeline` | `numeric_range` → `filter` → `windows` → fold |
| `adaptive` | `sort_n_adaptive` with tiny vs large merge buffer |
| `quadtree` | Point-cloud range query vs brute scan; nearest with refine |
| `dijkstra` | Decrease-key Dijkstra on `dynamic_pqueue` |
| `text` | `lines` / `starts_with` / `words` / `group_by` on a corpus |
| `combo` | In-place `next_combination` vs `next_permutation` |
