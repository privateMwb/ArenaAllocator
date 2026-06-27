# ArenaAllocator

[![C++26](https://img.shields.io/badge/C%2B%2B-26-blue)](https://en.cppreference.com/w/cpp/26)
[![Status](https://img.shields.io/badge/status-learning%20project-green)](https://github.com/privateMwb/ArenaAllocator)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A custom C++ arena allocator implementation built for learning low-level memory management, allocation strategies, frame-based lifetime control, and performance benchmarking.

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

ArenaAllocator (`Arena`) is a linear/bump allocator implemented from scratch in modern C++ (C++26).
It focuses on understanding how arena allocators work internally, including bump pointer allocation, frame-based lifetime management, and object lifecycle control.

It also includes:

- Typed allocation via `allocate<T>()`
- Object construction and destruction via `create<T>()` / `destroy<T>()`
- Frame stack for scoped memory lifetimes (`beginFrame` / `endFrame`)
- RAII frame management via `ArenaScope`
- Memory introspection via `owns()`, `view()`, and `getStats()`
- Benchmark suite comparing against heap (`new` / `delete`)
- Unit tests for correctness validation

---

## Motivation / Goals

This project was built to understand:

- Linear/bump pointer allocation strategies
- Frame-based memory lifetime management
- RAII patterns applied to allocator design
- Object lifecycle: construction and destruction without deallocation
- Memory introspection and ownership queries
- C++26 contracts for interface precondition enforcement
- Performance benchmarking vs heap allocation

---

## Features

- Bump pointer allocation with alignment support
- Typed allocation via `allocate<T>()`
- Object creation with forwarded constructor arguments via `create<T>()`
- Explicit destructor invocation via `destroy<T>()`
- Frame stack with configurable depth (`kMaxFrameDepth = 8`)
- RAII scoped frame management via `ArenaScope`
- Full reset to reclaim the entire buffer in O(1)
- Ownership query via `owns()`
- Live memory view via `view()` returning `std::span<const std::byte>`
- Debug statistics tracking via `getStats()`
- Move semantics with deleted copy
- C++26 contracts (`pre`) on all public functions

---

## Design Overview

Arena uses a single contiguous heap-allocated buffer with a bump pointer for allocation.

### Internal Structure

```
memory_ (pointer)
  ↓
[byte][byte][byte][byte][byte][...]
                   ↑
                offset_       cap_
```

- `memory_` → pointer to raw allocated buffer
- `cap_` → total buffer size in bytes
- `offset_` → current bump pointer position
- `frameStack_` → saved offsets for frame rewind
- `frameDepth_` → current frame stack depth
- `stats_` → debug allocation statistics

### Allocation Strategy

Allocation is a bump pointer advance with alignment:

```cpp
aligned = (offset_ + alignment - 1) & ~(alignment - 1);
ptr     = memory_ + aligned;
offset_ = aligned + size;
```

No per-object metadata. No free list. No deallocation overhead.

### Frame Management

Frames allow bulk rewind of the bump pointer:

```
beginFrame()   → push offset_ onto frameStack_
  allocate ...
endFrame()     → pop offset_ from frameStack_ (rewind)
```

Up to `kMaxFrameDepth` (8) nested frames are supported. Frames can be managed manually or automatically via `ArenaScope`.

### ArenaScope

`ArenaScope` is a `[[nodiscard]]` RAII wrapper that calls `beginFrame()` on construction and `endFrame()` on destruction:

```cpp
{
    AllocatorPro::ArenaScope scope{arena};
    // allocate temporary objects...
} // endFrame() called automatically
```

### Object Lifecycle

`create<T>()` allocates aligned memory and placement-constructs the object:

```cpp
T* obj = arena.create<T>(args...);
```

`destroy<T>()` invokes the destructor without deallocating:

```cpp
arena.destroy(obj);
```

Memory is reclaimed only via `reset()` or `endFrame()`.

### Memory Introspection

```
owns(ptr)   → true if ptr is within [memory_, memory_ + cap_)
view()      → std::span<const std::byte> over live portion [0, offset_)
getStats()  → reference to Stats struct tracking allocations and usage
```

### Exception Safety Model

- `allocate()` returns `nullptr` on failure — no exceptions
- `create()` returns `nullptr` if allocation fails
- Move operations are `noexcept`
- `reset()`, `beginFrame()`, `endFrame()` are `noexcept`
- C++26 `pre` contracts enforce caller obligations at function boundaries

---

## Complexity

### Time Complexity

| Operation       | Complexity | Notes                                      |
| --------------- | ---------- | ------------------------------------------ |
| `allocate`      | O(1)       | Bump pointer advance with alignment        |
| `allocate<T>`   | O(1)       | Typed wrapper over raw allocate            |
| `create<T>`     | O(1)       | Allocation + placement construction        |
| `destroy<T>`    | O(1)       | Destructor invocation only                 |
| `beginFrame`    | O(1)       | Push offset onto frame stack               |
| `endFrame`      | O(1)       | Pop and rewind bump pointer                |
| `reset`         | O(1)       | Zero the offset — no per-object work       |
| `owns`          | O(1)       | Pointer bounds check                       |
| `view`          | O(1)       | Span construction over live buffer         |
| `getStats`      | O(1)       | Reference return                           |

### Space Complexity

- O(n) for the backing buffer
- O(kMaxFrameDepth) for the frame stack (fixed at 8)
- O(1) for all other metadata

### Notes

- No per-allocation overhead — no headers, no free list, no metadata
- Memory is never released per-object; only `reset()` or `endFrame()` reclaim space
- `destroy<T>()` invokes the destructor but does not reduce `offset_`

---

## Quick Example

### Basic Allocation and Reset

```cpp
#include "Arena.h"

using namespace AllocatorPro;

int main() {
    Arena arena{1024};

    void* p1 = arena.allocate(64, alignof(std::max_align_t));
    void* p2 = arena.allocate(128, alignof(std::max_align_t));

    int*    pi = arena.allocate<int>();
    double* pd = arena.allocate<double>();

    arena.reset(); // reclaim everything in O(1)
}
```

### Object Lifecycle

```cpp
#include "Arena.h"

using namespace AllocatorPro;

struct Entity {
    int id_; const char* name_;
    Entity(int id, const char* name) : id_(id), name_(name) {}
    ~Entity() {}
};

int main() {
    Arena arena{2048};

    Entity* e = arena.create<Entity>(1, "Warrior");

    arena.destroy(e); // destructor called, memory stays in arena

    arena.reset();    // reclaim all memory
}
```

### Frame Management with ArenaScope

```cpp
#include "Arena.h"
#include "ArenaScope.h"

using namespace AllocatorPro;

int main() {
    Arena arena{4096};

    int* persistent = arena.create<int>(42); // persists across frames

    {
        ArenaScope scope{arena}; // beginFrame()

        int* temp1 = arena.create<int>(1);
        int* temp2 = arena.create<int>(2);

    } // endFrame() — temp1 and temp2 reclaimed automatically

    arena.reset();
}
```

---

## Core API

### Constructors

```cpp
Arena arena{size};              // allocates backing buffer of `size` bytes
Arena b{std::move(a)};         // move construction — transfers ownership
b = std::move(a);              // move assignment — transfers ownership
```

### Core Allocation

```cpp
void* allocate(std::size_t size, std::size_t alignment) noexcept;

template<typename T>
T* allocate() noexcept;
```

### Object Lifecycle

```cpp
template<typename T, typename... Args>
T* create(Args&&... args);

template<typename T>
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
bool owns(const void* ptr) const noexcept;

std::span<const std::byte> view() const noexcept;

const Stats& getStats() const noexcept;

std::size_t capacity()  const noexcept;
std::size_t used()      const noexcept;
std::size_t remaining() const noexcept;
```

### ArenaScope

```cpp
AllocatorPro::ArenaScope scope{arena}; // beginFrame on construct, endFrame on destruct
```

---

## Benchmark Results

Benchmarks compare `Arena` against heap (`new` / `delete`) across all operations.
All times are total elapsed time for the listed iteration count.

> Compiled with `-std=c++26`. Results may vary depending on hardware and compiler optimizations.

### Constructor

```
----------------------------------------------------------------------
Constructor Benchmarks                  Time           Iterations
----------------------------------------------------------------------
Arena Construct                         317.94 ms       1000000
Heap Construct                          222.38 ms       1000000

Arena Move Construct                    330.35 ms       1000000
Heap Move Construct                     293.86 ms       1000000

Arena Move Assign                       614.78 ms       1000000
Heap Move Assign                        277.47 ms       1000000
----------------------------------------------------------------------
```

### Core Allocation

```
----------------------------------------------------------------------
Core Allocation Benchmarks              Time           Iterations
----------------------------------------------------------------------
Arena Raw Allocate                      166.82 ms       1000000
Heap Raw Allocate                       227.29 ms       1000000

Arena Typed Allocate                    145.39 ms       500000
Heap Typed Allocate                     89.55 ms        500000

Arena Create                            148.98 ms       500000
Heap Create                             228.44 ms       500000

Arena Sequential                        57.41 ms        100000
Heap Sequential                         348.28 ms       100000

Arena Until Full                        43.69 ms        100000
Heap Until Full                         292.47 ms       100000
----------------------------------------------------------------------
```

### Object Lifecycle

```
----------------------------------------------------------------------
Object Lifecycle Benchmarks             Time           Iterations
----------------------------------------------------------------------
Arena Create                            259.20 ms       1000000
Heap Create                             169.78 ms       1000000

Arena Destroy                           308.61 ms       1000000
Heap Destroy                            120.75 ms       1000000

Arena Create Destroy Cycle              145.39 ms       500000
Heap Create Destroy Cycle               108.97 ms       500000

Arena Multiple Creates                  28.37 ms        100000
Heap Multiple Creates                   319.88 ms       100000

Arena Create Probe                      91.48 ms        500000
Heap Create Probe                       61.36 ms        500000
----------------------------------------------------------------------
```

### Frame Management

```
----------------------------------------------------------------------
Frame Management Benchmarks             Time           Iterations
----------------------------------------------------------------------
Arena Begin End Frame                   170.01 ms       1000000
Heap Begin End Frame                    141.98 ms       1000000

Arena Scope                             169.22 ms       1000000
Heap Scope                              226.52 ms       1000000

Arena Nested Frames                     168.76 ms       500000
Heap Nested Frames                      458.88 ms       500000

Arena Frame With Allocs                 31.19 ms        100000
Heap Frame With Allocs                  308.18 ms       100000

Arena Scope With Allocs                 44.73 ms        100000
Heap Scope With Allocs                  259.05 ms       100000
----------------------------------------------------------------------
```

### State Management

```
----------------------------------------------------------------------
State Management Benchmarks             Time           Iterations
----------------------------------------------------------------------
Arena Reset Single                      319.70 ms       1000000
Heap Reset Single                       123.63 ms       1000000

Arena Reset Multiple                    113.59 ms       500000
Heap Reset Multiple                     792.59 ms       500000

Arena Reset Reallocate                  141.48 ms       500000
Heap Reset Reallocate                   280.73 ms       500000

Arena Repeated Cycles                   19.79 ms        100000
Heap Repeated Cycles                    99.35 ms        100000
----------------------------------------------------------------------
```

### Introspection

```
----------------------------------------------------------------------
Introspection Benchmarks                Time           Iterations
----------------------------------------------------------------------
Arena Owns                              2.65 ms         1000000
Heap Owns                               324.12 ms       1000000

Arena View                              2.63 ms         1000000
Heap View                               1.07 ms         1000000

Arena Get Stats                         2.11 ms         1000000
Heap Get Stats                          1.07 ms         1000000

Arena Capacity Used Remaining           5.80 ms         1000000
Heap Capacity Used Remaining            526.46 us       1000000
----------------------------------------------------------------------
```

### Summary

Arena dominates wherever bulk patterns or batch lifetimes are involved.
Sequential allocation (`Arena Sequential`: 57 ms vs `Heap Sequential`: 348 ms) and
filling to capacity (`Arena Until Full`: 43 ms vs `Heap Until Full`: 292 ms) show
the clearest wins — no per-allocation overhead means the gap widens with count.

Frame management tells the same story. Nested frames (`Arena Nested Frames`: 168 ms vs
`Heap Nested Frames`: 458 ms) and frame/scope with allocations show 6–10x advantages
because `endFrame` is a single pointer rewind vs N individual `delete` calls.

State management reinforces this — `reset` against multiple allocations
(`Arena Reset Multiple`: 113 ms vs `Heap Reset Multiple`: 792 ms) is the starkest
example of O(1) bulk reclaim vs O(n) heap deallocation.

Heap wins on single object operations. Single `create`/`destroy` cycles
(`Arena Create`: 259 ms vs `Heap Create`: 169 ms) favor heap because the arena
still pays construction cost without gaining anything from its bulk advantages.
Introspection accessors (`owns`, `view`, `getStats`) are near-zero cost on both sides.

| Category             | Winner | Notes                                          |
| -------------------- | ------ | ---------------------------------------------- |
| Raw allocation       | Arena  | Bump pointer vs heap search                    |
| Sequential creates   | Arena  | 6x faster — no per-object overhead             |
| Bulk fill            | Arena  | 6x faster — single buffer, no fragmentation    |
| Frame with allocs    | Arena  | 10x faster — endFrame vs N deletes             |
| Scope with allocs    | Arena  | 5x faster — RAII rewind vs N deletes           |
| Nested frames        | Arena  | 2.7x faster — stack rewind vs heap dealloc     |
| Reset multiple       | Arena  | 7x faster — O(1) rewind vs O(n) delete         |
| Single create        | Heap   | Arena pays construction with no bulk benefit   |
| Single destroy       | Heap   | Heap dealloc faster without frame advantage    |
| Introspection        | Tie    | Both near-zero cost                            |

**Use Arena when:** lifetimes are grouped, objects are short-lived, or bulk reset is needed.
**Use heap when:** individual objects have independent lifetimes and are few in number.

---

## Project Structure

```
ArenaAllocator/
├── include/
│   ├── Arena.h
│   ├── Arena.tpp
│   └── ArenaScope.h
│
├── src/
│   └── Arena.cpp
│
├── tests/
│   ├── test_helper.h
│   ├── test_all.cpp
│   ├── test_constructor.cpp
│   ├── test_core_allocation.cpp
│   ├── test_object_lifecycle.cpp
│   ├── test_frame_management.cpp
│   ├── test_state_management.cpp
│   └── test_introspection.cpp
│
├── benchmarks/
│   ├── benchmark_helper.h
│   ├── bench_all.cpp
│   ├── bench_constructor.cpp
│   ├── bench_core_allocation.cpp
│   ├── bench_object_lifecycle.cpp
│   ├── bench_frame_management.cpp
│   ├── bench_state_management.cpp
│   └── bench_introspection.cpp
│
├── examples/
│   ├── example_helper.h
│   ├── example_basic.cpp
│   ├── example_lifecycle.cpp
│   └── example_scope.cpp
│
├── CMakeLists.txt
├── README.md
└── LICENSE
```

---

## Build Instructions

### Requirements

- GCC 16+ or Clang with C++26 support
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
./ex1   # example_basic
./ex2   # example_lifecycle
./ex3   # example_scope
```

---

## Notes

- Arena memory is never released per-object. Reclaim via `reset()` or `endFrame()`.
- `destroy<T>()` calls the destructor but does not rewind the bump pointer.
- `ArenaScope` is `[[nodiscard]]` — a discarded temporary would immediately call `endFrame()`.
- C++26 contracts (`pre`) are active on all public functions. GCC 16 support is experimental.
- The frame stack depth is fixed at `kMaxFrameDepth = 8`. Exceeding it is a contract violation.

---

## License

[MIT](LICENSE) — free to use, modify, and distribute for educational and personal purposes.
