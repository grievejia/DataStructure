# DataStructure
This repo is a collection of data structure I used frequently but are not included in the STL.

Here's a list of the included contents:
* `ArrayRef`, a non-owning view of arrays and vectors. Will be superceded once the range library gets into the C++ standard.
* `DenseMap`, a very efficient hash map implementation copied from LLVM codebase.
* `DenseSet`, the set version of DenseMap.
* `DynamicBitSet`, a sane alternative to `std::vector<bool>`, copied from Boost codebase.
* `StringView`, a non-owning view of string types. Will be superceded by `std::string_view` once C++17 is out.
* `StringMap`, a DenseMap with owning string as keys. It also supports heterogeneous lookup with StringView as lookup-key. This one is copied from LLVM.
* `VectorMap`, a map-like data structure implemented with a sorted vector. 
* `VectorSet`, the set version of VectorMap.
* `UnorderedCollection`, an owning, stable, unordered container that supports back insertion, deletion, and iteration.
* `FIFOWorkList`, a first-in-first-out queue that can automatically remove duplicates.
* `PriorityWorkList`, a priority queue that can automatically remove duplicates.
* `UnorderedWorkList`, an efficient worklist implemented by two swapping `std::vector`s. Note that it will tolerate duplicated elements.
