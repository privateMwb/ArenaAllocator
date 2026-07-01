# Arena Allocator

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![Status](https://img.shields.io/badge/status-learning%20project-green)](https://github.com/privateMwb/ArenaPro)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A custom C++ arena allocator implementation built for learning low-level memory management, bump pointer allocation strategies, nested frame-based rollback, and performance benchmarking.

---

## Table of Contents

- [Overview](#overview)
- [Motivation / Goals](#motivation--goals)
- [Features](#features)
- [Design Overview](#design-overview)
- [Complexity](#complexity)
- [Quick Example](#quick-example)
- [Core API](#core-api)
- [Benchmark Results](#benchmark-results)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Notes](#notes)
- [License](#license)

---

## Overview

ArenaPro (`Arena`) is a linear bump pointer allocator implemented from scratch in modern C++ (C++23).
It focuses on understanding how arena allocators work internally, including sequential bump pointer allocation, nested frame-based rollback, aligned memory management, and RAII scope guards.

It also includes:

- Typed object construction and destruction via `create<T>()` / `destroy<T>()`
- Typed storage allocation via `allocate<T>()`
- Nested frame rollback via `beginFrame()` / `endFrame()`
- RAII scope guard via `ArenaScope`
- O(1) bulk reset via `reset()`
- Ownership queries via `owns()`
- Read-only memory view via `view()`
- Frame depth introspection via `frameDepth()`
- Optional debug statistics via `Arena<true>`
- Benchmark suite comparing against heap (`new` / `delete`)
- Unit tests for correctness validation

---

## Motivation / Goals

This project was built to understand:

- Linear bump pointer allocation strategies
- Nested frame-based rollback without per-object metadata
- Aligned memory management using `::operator new`
- Object lifecycle: construction and destruction within an arena
- RAII scope guard patterns for automatic frame unwinding
- Optional compile-time statistics with zero overhead when disabled
- Performance benchmarking vs heap allocation

---

## Features

- O(1) allocation via bump pointer
- O(1) frame rollback via offset restoration
- Aligned allocation with configurable default and per-request alignment
- Typed storage allocation via `allocate<T>()` — correctly aligned, uninitialized
- Typed object creation with forwarded constructor arguments via `create<T>()`
- Explicit destructor invocation via `destroy<T>()`
- RAII scope guard via `ArenaScope` for automatic frame rollback
- Nested frame stack with configurable maximum depth (`kMaxFrameDepth_ = 8`)
- Full reset to reclaim all memory in O(1) without calling destructors
- Ownership query via `owns()`
- Read-only memory view via `view()` returning `std::span<const std::byte>`
- Frame depth introspection via `frameDepth()`
- Optional debug statistics via `Arena<true>` with zero overhead when disabled
- `[[no_unique_address]]` on stats storage — no size penalty when stats are off
- Move semantics with deleted copy
- `std::constructible_from` concept constraint on `create<T>()`

---

## Design Overview

Arena uses a single contiguous heap-allocated slab with a bump pointer for allocation and a fixed-depth frame stack for nested rollback.

### Internal Structure

```
memory_ (pointer)
  ↓
[alloc 0][alloc 1][padding][alloc 2][alloc 3][...]
                                              ↑
                                           offset_
```

- `memory_`     → pointer to raw allocated slab
- `cap_`        → total capacity in bytes
- `offset_`     → current bump pointer position
- `alignShift_` → log2 of the default alignment
- `frameStack_` → fixed-depth array of saved offsets
- `frameDepth_` → number of currently open frames
- `stats_`      → optional debug statistics (zero-size when disabled)

### Allocation Strategy

Allocation aligns the current offset and bumps it forward:

```cpp
const std::size_t aligned   = alignForward(offset_, toShift(requestAlignment));
std::byte*        ptr       = memory_ + aligned;
offset_                     = aligned + size;
return ptr;
```

No heap traffic after construction. No per-allocation metadata.

### Frame System

`beginFrame()` pushes the current offset onto an internal stack; `endFrame()` pops it:

```cpp
arena.beginFrame();    // save current position
// ... allocations ...
arena.endFrame();      // restore to saved position
```

All memory allocated after `beginFrame()` is reclaimed in O(1) on `endFrame()`.
The frame stack enforces strict LIFO ordering and is capped at `kMaxFrameDepth_ = 8`.

### ArenaScope

`ArenaScope` is a RAII guard that calls `beginFrame()` on construction and `endFrame()` on destruction:

```cpp
{
    ArenaScope scope{arena};
    // ... allocations automatically rolled back on scope exit ...
}
```

Nested scopes unwind in LIFO order naturally.

### Alignment Strategy

Alignment is stored as a bit shift (`alignShift_`) rather than a raw value.
This turns alignment math into a bitmask operation with no division:

```cpp
const std::size_t mask = (std::size_t{1} << shift) - 1u;
return (ptr + mask) & ~mask;
```

`toShift()` uses `std::countr_zero` (C++20) for a branchless, constexpr-friendly conversion.

### Object Lifecycle

`create<T>()` allocates aligned memory and placement-constructs the object:

```cpp
T* obj = arena.create<T>(args...);
```

`destroy<T>()` invokes the destructor without releasing memory:

```cpp
arena.destroy(obj);   // destructor called, memory remains in arena
```

### Optional Statistics

Statistics are controlled at compile time via the `EnableStats` template parameter:

```cpp
Arena<false> arena{1024};   // no stats — zero overhead
Arena<true>  debug{1024};   // stats enabled
```

`[[no_unique_address]]` ensures the stats struct occupies zero bytes when disabled.

The `statReset()` / `statDealloc()` split separates full stat clearing (`reset()`) from
point-in-time usage updates (`endFrame()`), preserving lifetime counters across frame rollbacks.

### Exception Safety Model

- `allocate()` returns `nullptr` on exhaustion — no exceptions
- `create<T>()` returns `nullptr` if allocation fails
- Move operations are `noexcept`
- `reset()`, `beginFrame()`, `endFrame()`, `destroy<T>()` are `noexcept`
- `beginFrame()` is precondition-guarded — frame depth must be less than `kMaxFrameDepth_`
- `endFrame()` is precondition-guarded — at least one frame must be open

---

## Complexity

### Time Complexity

| Operation      | Complexity | Notes                                   |
| -------------- | ---------- | --------------------------------------- |
| `allocate`     | O(1)       | Bump pointer + alignment mask           |
| `allocate<T>`  | O(1)       | Typed wrapper — fully inlined           |
| `beginFrame`   | O(1)       | Offset push onto fixed-depth stack      |
| `endFrame`     | O(1)       | Offset pop and restore                  |
| `create<T>`    | O(1)       | Allocation + placement construction     |
| `destroy<T>`   | O(1)       | Destructor invocation only              |
| `reset`        | O(1)       | Offset and frame depth reset to zero    |
| `owns`         | O(1)       | Bounds check                            |
| `view`         | O(1)       | Span construction over used range       |
| `frameDepth`   | O(1)       | Member return                           |
| `getStats`     | O(1)       | Reference return                        |

### Space Complexity

- O(n) for the backing slab (`size` bytes)
- O(1) for all metadata
- O(0) for stats when `EnableStats = false`

### Notes

- No per-allocation overhead — frame-based rollback requires no metadata per block
- `reset()` does not call destructors — caller is responsible for object cleanup
- `endFrame()` reclaims all memory allocated since `beginFrame()` in a single assignment
- Alignment padding may consume bytes between allocations depending on request alignment

---

## Quick Example

### Basic Allocation

```cpp
#include <ArenaPro/Arena.h>

using namespace AllocatorPro;

int main() {
    Arena<> arena{1024};

    std::byte* a = arena.allocate(128);
    std::byte* b = arena.allocate(256);
    (void)a;
    (void)b;

    arena.reset();   // reclaim all memory in O(1)
}
```

### Frame-Based Rollback

```cpp
#include <ArenaPro/Arena.h>

using namespace AllocatorPro;

int main() {
    Arena<> arena{1024};

    (void)arena.allocate(128);

    arena.beginFrame();          // save current position
    (void)arena.allocate(256);
    arena.endFrame();            // restore to saved position
}
```

### RAII Scope Guard

```cpp
#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>

using namespace AllocatorPro;

int main() {
    Arena<> arena{1024};
    (void)arena.allocate(128);

    {
        ArenaScope<false> scope{arena};
        (void)arena.allocate(256);
    }   // automatically rolled back on scope exit
}
```

### Object Lifecycle

```cpp
#include <ArenaPro/Arena.h>

using namespace AllocatorPro;

struct Particle {
    float x_, y_, z_;
    Particle(float x, float y, float z) : x_(x), y_(y), z_(z) {}
    ~Particle() {}
};

int main() {
    Arena<> arena{1024};

    Particle* p = arena.create<Particle>(1.0f, 2.0f, 3.0f);
    arena.destroy(p);   // destructor called, memory remains in arena
    arena.reset();      // reclaim all memory in O(1)
}
```

### Debug Statistics

```cpp
#include <ArenaPro/Arena.h>

using namespace AllocatorPro;

int main() {
    Arena<true> arena{1024};

    (void)arena.allocate(128);
    (void)arena.allocate(256);

    const auto& stats = arena.getStats();
    // stats.allocations_, stats.totalAllocated_, stats.peakUsed_, stats.currentUsed_
}
```

---

## Core API

### Constructors

```cpp
Arena<> arena{size};                    // default alignment
Arena<> arena{size, alignment};         // custom alignment
Arena<> b{std::move(a)};               // move construction
b = std::move(a);                      // move assignment
```

### Memory Management

```cpp
[[nodiscard]] std::byte* allocate(std::size_t size,
    std::size_t request_alignment = alignof(std::max_align_t)) noexcept;

template<typename T>
[[nodiscard]] T* allocate() noexcept;
```

### Object Lifecycle

```cpp
template<typename T, typename... Args>
requires (!std::is_array_v<T>) && std::constructible_from<T, Args...>
[[nodiscard]] T* create(Args&&... args);

template<typename T>
requires (!std::is_array_v<T>)
void destroy(T* ptr) noexcept;
```

### Frame Management

```cpp
void beginFrame() noexcept;
void endFrame()   noexcept;
```

### State Management

```cpp
void reset() noexcept;
```

### Introspection

```cpp
[[nodiscard]] bool owns(const void* ptr)              const noexcept;
[[nodiscard]] std::span<const std::byte> view()       const noexcept;

[[nodiscard]] const Stats& getStats() const noexcept requires EnableStats;

[[nodiscard]] std::size_t used()        const noexcept;
[[nodiscard]] std::size_t remaining()   const noexcept;
[[nodiscard]] std::size_t capacity()    const noexcept;
[[nodiscard]] std::size_t frameDepth()  const noexcept;
```

### ArenaScope

```cpp
ArenaScope<false> scope{arena};   // calls beginFrame on construction
                                  // calls endFrame on destruction
```

---

## Benchmark Results

Benchmarks compare `Arena` against heap (`new` / `delete`) across all operations.
All times are total elapsed time for the listed iteration count.

> Compiled with `-std=c++23`. Results may vary depending on hardware and compiler optimizations.

### Allocation

```
----------------------------------------------------------------------
Allocation Benchmarks                   Time           Iteration
----------------------------------------------------------------------
Arena Allocate                          3.56 ms         1000000
Heap Allocate                           170.60 ms       1000000

Arena Allocate Aligned                  3.28 ms         1000000
Heap Allocate Aligned                   164.52 ms       1000000

Arena Sequential                        6.15 ms         1000000
Heap Sequential                         388.09 ms       1000000
----------------------------------------------------------------------
```

### Lifecycle

```
----------------------------------------------------------------------
Object Lifecycle Benchmarks             Time           Iteration
----------------------------------------------------------------------
Arena Create Trivial                    5.87 ms         1000000
Heap Create Trivial                     169.09 ms       1000000

Arena Create Non Trivial                5.90 ms         1000000
Heap Create Non Trivial                 130.79 ms       1000000
----------------------------------------------------------------------
```

### Reset

```
----------------------------------------------------------------------
Reset Benchmarks                        Time           Iteration
----------------------------------------------------------------------
Arena Exhaustion Reset                  60.43 ms        1000000
Heap Exhaustion Reset                   2.08 s          1000000

Reset Stats Disabled                    60.01 ms        1000000
Reset Stats Enabled                     58.90 ms        1000000
----------------------------------------------------------------------
```

### Scope

```
----------------------------------------------------------------------
ArenaScope Benchmarks                   Time           Iteration
----------------------------------------------------------------------
Arena Scope                             3.26 ms         1000000
Arena Raw Frame                         3.26 ms         1000000

Arena Scope Alloc                       3.25 ms         1000000
Heap Alloc                              178.67 ms       1000000

Arena Nested Scope                      8.71 ms         1000000
Arena Nested Raw Frame                  8.67 ms         1000000
----------------------------------------------------------------------
```

### Summary

Arena dominates heap across every allocation pattern due to its O(1) bump pointer strategy
and zero heap traffic after construction.

Single allocation (`Arena Allocate`: 3.56 ms vs `Heap Allocate`: 170.60 ms) shows a 48x
advantage — a bump pointer increment and mask vs heap search with fragmentation overhead.

Aligned allocation (`Arena Allocate Aligned`: 3.28 ms vs `Heap Allocate Aligned`: 164.52 ms)
shows a 50x advantage — alignment is a single bitmask operation stored as a bit shift,
with no additional cost over unaligned allocation.

Sequential allocations (`Arena Sequential`: 6.15 ms vs `Heap Sequential`: 388.09 ms)
shows a 63x advantage — contiguous bump pointer vs three independent heap searches
with fragmentation overhead per call.

Exhaustion and reset (`Arena Exhaustion Reset`: 60.43 ms vs `Heap Exhaustion Reset`: 2.08 s)
shows a 34x advantage — `reset()` is a single offset assignment and frame depth clear
vs N individual heap deletes regardless of block count.

Scope overhead (`Arena Scope`: 3.26 ms vs `Arena Raw Frame`: 3.26 ms) is zero —
the RAII wrapper adds no measurable cost compared to manual `beginFrame`/`endFrame`,
both vastly outperforming heap at 178.67 ms.

Stats overhead (`Reset Stats Disabled`: 60.01 ms vs `Reset Stats Enabled`: 58.90 ms)
is negligible — the `if constexpr (EnableStats)` branch eliminates all stat overhead
at compile time when disabled, and the enabled path adds no measurable cost.

Non-trivial lifecycle (`Arena Create Non Trivial`: 5.90 ms vs
`Heap Create Non Trivial`: 130.79 ms) shows a 22x advantage — the arena
eliminates heap search entirely, paying only construction and destruction cost.

| Category               | Winner | Notes                                              |
| ---------------------- | ------ | -------------------------------------------------- |
| Single allocate        | Arena  | 48x faster — bump pointer vs heap search           |
| Aligned allocate       | Arena  | 50x faster — bitmask alignment vs heap overhead    |
| Sequential allocations | Arena  | 63x faster — contiguous bump vs N heap calls       |
| Exhaustion reset       | Arena  | 34x faster — single assignment vs N heap deletes   |
| Trivial lifecycle      | Arena  | 29x faster — no heap traffic after construction    |
| Non-trivial lifecycle  | Arena  | 22x faster — arena eliminates heap search entirely |
| Scope vs raw frame     | Arena  | Zero RAII overhead — 3.26 ms vs 3.26 ms            |
| Stats disabled vs on   | Arena  | Negligible overhead — compile-time elimination     |

**Use Arena when:** allocation is linear, memory has natural frame lifetimes, or bulk reset patterns are needed.
**Use heap when:** object lifetimes are independent, sizes vary unpredictably, or arbitrary deallocation order is required.

---

## Project Structure

```
ArenaPro/
├── include/
│   └── ArenaPro/
│       ├── Contract.h
│       ├── Arena.h
│       ├── Arena.tpp
│       └── ArenaScope.h
│
├── tests/
├── benchmarks/
├── examples/
│
├── cmake/
│   └── ArenaProConfig.cmake.in
│
├── .gitignore
├── CMakeLists.txt
├── README.md
└── LICENSE
```

---

## Build Instructions

### Requirements

- GCC 16+ or Clang with C++23 support
- CMake 3.20+

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run Tests

```bash
./tests
```

### Run Benchmarks

```bash
./benchmarks
```

### Run Examples

```bash
./example_basic
./example_lifecycle
./example_scope
./example_stats
```

---

## Notes

- `reset()` does not call destructors. Caller is responsible for destroying live objects before reset.
- `endFrame()` reclaims all memory allocated since `beginFrame()` — objects in that range are not destroyed.
- `destroy<T>()` calls the destructor but does not release memory — use `endFrame()` or `reset()` to reclaim.
- `getStats()` is only callable on `Arena<true>` — calling it on `Arena<false>` is a compile error.
- `ArenaScope` calls `beginFrame()` at construction — allocations made before the scope are preserved.
- The frame stack is capped at `kMaxFrameDepth_ = 8` — exceeding this depth triggers a precondition failure.
- Alignment padding between allocations may cause `used()` to exceed the sum of requested sizes.
- `reset()` fully clears all statistics including lifetime counters — `endFrame()` only updates `currentUsed_`.

---

## License

[MIT](LICENSE) — free to use, modify, and distribute for educational and personal purposes.
