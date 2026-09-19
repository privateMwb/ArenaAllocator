# Changelog

All notable changes to ArenaAllocator are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Differential fuzz harness (`fuzz/fuzz_arena.cpp`) run through
  ClusterFuzzLite under AddressSanitizer and UndefinedBehaviorSanitizer:
  a short pass on every pull request that touches the library, plus a
  longer nightly batch run. See `FUZZING.md`.

## [1.0.0] - 2026-07-28

The first stable release of ArenaAllocator, a fixed-capacity, bump-pointer
memory arena for modern C++20.

### Added
- O(1) bump-pointer allocation via `allocate()`, including a typed
  `allocate<T>()` overload.
- In-place construction and destruction of arbitrary types via
  `create<T>()` / `destroy<T>()`.
- Nested frame stack (`beginFrame()` / `endFrame()`) for stack-like, LIFO
  rollback of allocations.
- `ArenaScope`, an RAII guard that opens a frame on construction and
  rolls it back on destruction, including when an exception unwinds
  the scope.
- `reset()` for O(1) bulk reclamation of the entire buffer.
- Alignment-aware allocation, including over-aligned types.
- Optional allocation statistics (total allocated, current usage, peak
  usage, allocation count), enabled through the compile-time
  `EnableStats` flag.
- Introspection via `capacity()`, `used()`, `remaining()`, `frameDepth()`,
  `view()`, and `owns()`.
- Move construction and move assignment.
- Contract-based (assert-driven) precondition enforcement, documented
  consistently across the API.
- Exception safety: allocation and constructor failures propagate without
  corrupting arena state.
- `rain::` namespace alias for the library's public types.

### Performance
- A single upfront buffer allocation eliminates per-allocation heap traffic.
- Bump-pointer allocation keeps `allocate()` / `create()` to a handful of
  arithmetic operations.
- Frame-based rollback reclaims an entire batch of allocations in O(1),
  without touching each one individually.
- `reset()` reclaims the whole buffer in O(1), regardless of how many
  objects were allocated.
- Exhaustion is a plain bounds check and a `nullptr` return: no
  exceptions, no reallocation.
- Alignment is resolved via bit-shift arithmetic rather than a general
  modulo/division path.
- Statistics tracking compiles away entirely when `EnableStats` is
  `false`, so a plain arena carries no bookkeeping cost.
- Benchmarked against `stdArena` (a naive `new[]` + linear-scan baseline)
  at 10K / 100K / 1M iterations; largest wins on the exhaustion path,
  `reset()` + refill, and large or over-aligned allocations. The trade-off
  is a slower `Construction`, since the buffer is allocated eagerly and
  aligned up front. Full results in `benchmarks/results/v1_0_0.md`.

### Testing
- Comprehensive test suite covering unit, integration, lifecycle, and
  regression tests; move semantics; exception safety; allocation failure
  handling; frame-based rollback and nested frames; `ArenaScope` RAII
  behavior; alignment behavior; statistics tracking; and external
  synchronization contracts.
- 99.1% line coverage and 100.0% function coverage, excluding test
  infrastructure and third-party dependencies.

### CI
- Automated builds and tests across GCC, Clang, MSVC, and AppleClang, each
  in Debug and Release configurations.

[Unreleased]: https://github.com/privateMwb/ArenaAllocator/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/privateMwb/ArenaAllocator/releases/tag/v1.0.0
