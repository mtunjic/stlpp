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



---

<img src="doc/stepanopv.jpg" alt="Alexander Stepanov" width="140">

For Alexander Stepanov.
