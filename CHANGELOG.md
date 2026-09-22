# Changelog

## Unreleased

### Fixed — 2026-09-22

- Preserve insertion order in balanced reductions with associative, noncommutative operations.
- Correct in-place exclusive scan, buffered partition boundaries, and equal-key ordering in backward merge.
- Materialize values instead of retaining mutable iterator proxies in scans, reductions, insertion sort, and permutation cycles.
- Support distinct iterator/sentinel types in quantifiers, span/break, merge/set operations, parallel map/find/reduce, adjacent difference, partition copy, Cartesian products, flatten/mapcat, and sorted sums/search.
- Avoid concurrent writes to packed proxy outputs such as `vector<bool>` in map; support mutable mapping callables and result types without default constructors.
- Prevent arithmetic overflow in large strides, numeric iterator distances, narrow counted bounds, modular multiplication with the minimum signed modulus, and the final increment of iota.
- Fall back to comparison sorting when finite bucket-sort inputs produce a nonfinite or zero double-precision span.
- Treat nonpositive counted-sort and unary-power requests as no-ops; require forward iterators for take/take_while, whose returned bounds require multiple passes.
- Deep-copy binary-tree node links, leave moved-from trees and ring buffers empty, and restore root parent links after rotating traversal.
- Support move-only hash-table values during growth, preserve copyable entries on rehash failure, clean up failed slot construction, and keep moved-from table sizes consistent. Rehash failure for noncopyable types with throwing moves leaves a valid empty table.
- Terminate quadtree subdivision at floating-point resolution and create child nodes transactionally with overflow-safe midpoints.
- Preserve temporary inner-range lifetimes when flatten iterators are copied; support const inner ranges and distinct inner sentinels. Temporary inner ranges expose input-iterator semantics.
- Roll back priority-queue handle bookkeeping when insertion fails and allocate free-handle storage before removing an element.
- Keep aliased search/replacement views valid during global string replacement, reject the null character as a vowel, and avoid division by zero when printing a priority queue with zero line width.
- Correct backward-copy overlap and jagged-transpose documentation.


## Historical modernization note

The algorithms.h file has been successfully modernized using C++20 features from the doc\modern-cpp-features directory. Here's a summary of the changes:

Key Updates:

find_if_n and find_backward_if:
Replaced outdated iterator types with std::input_iterator and std::bidirectional_iterator.
Simplified logic using std::ranges::find_if for range-based iteration.
Applied C++20 concepts like std::integral and std::predicate for type safety.
