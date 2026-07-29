# ArenaAllocator

<p align="center">
  <img src="https://img.shields.io/github/v/release/privateMwb/ArenaAllocator?style=for-the-badge&logo=github&color=yellow" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-orange?style=for-the-badge" alt="License - MIT">
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue?style=for-the-badge&logo=c%2B%2B" alt="C++ - 20">
</p>

<p align="center">
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/build.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/build.yml/badge.svg" alt="Build and Test">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/benchmark.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/benchmark.yml/badge.svg" alt="Benchmarks">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/coverage.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/coverage.yml/badge.svg" alt="Coverage">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/sanitizers.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/sanitizers.yml/badge.svg" alt="Sanitizers">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/clang-tidy.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/clang-tidy.yml/badge.svg" alt="Clang Tidy">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/clang-format.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/clang-format.yml/badge.svg" alt="Clang Format">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/docs.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/release.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/release.yml/badge.svg" alt="Release">
  </a>
  <a href="https://github.com/privateMwb/ArenaAllocator/actions/workflows/packaging.yml">
    <img src="https://github.com/privateMwb/ArenaAllocator/actions/workflows/packaging.yml/badge.svg" alt="Packaging">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/GCC-support-B46F1B?style=flat&logo=gnu" alt="GCC - support">
  <img src="https://img.shields.io/badge/Clang-support-045891?style=flat&logo=llvm" alt="Clang - support">
  <img src="https://img.shields.io/badge/MSVC-support-5C2D91?style=flat" alt="MSVC - support">
  <img src="https://img.shields.io/badge/AppleClang-support-000000?style=flat&logo=apple" alt="AppleClang - support">
</p>

ArenaAllocator is a header-only, fixed-capacity bump-pointer memory arena for modern C++ — O(1) `allocate()`/`create()`, a single buffer allocated once at construction instead of per-allocation heap traffic, and stack-like frame rollback via `ArenaScope` instead of tracking and freeing objects one at a time.

## 📑 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Development](#development)
- [Benchmarks](#benchmarks)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

## <a id="features"></a>✨ Features

- **Single upfront buffer, bump-pointer allocation** — the whole arena is allocated once at construction; `allocate()`/`create<T>()` are just a few arithmetic operations advancing a cursor, so steady-state allocation never touches the heap again.
- **Frame-based rollback via `ArenaScope`** — `beginFrame()`/`endFrame()` push and pop a checkpoint on an internal frame stack; `ArenaScope` wraps that in RAII so scratch allocations are automatically reclaimed — even when an exception unwinds through the scope — without freeing anything individually.
- **In-place construction and destruction** — `create<T>()` forwards its arguments directly into `T`'s constructor inside the arena; `destroy<T>()` runs `T`'s destructor without moving the cursor, since the arena reclaims space in bulk (`endFrame()`/`reset()`), not per object.
- **Alignment-aware, contract-based API** — `allocate()` honors an explicit alignment, including over-aligned types, and preconditions across the API are documented and enforced via assert-based contracts, consistent everywhere rather than mixed error-handling styles.
- **Optional, zero-cost statistics** — a compile-time `EnableStats` flag adds allocation/usage tracking (`getStats()`) with zero overhead when disabled.

## <a id="requirements"></a>📋 Requirements

- A C++20-conformant compiler (tested: GCC, Clang, MSVC, AppleClang)
- CMake 3.20+

## <a id="installation"></a>📦 Installation

**From source:**

```bash
git clone https://github.com/privateMwb/ArenaAllocator.git
cd ArenaAllocator
cmake -B build \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_REGRESSION=OFF \
  -DBUILD_EXAMPLES=OFF
cmake --install build
```

Then, in your own `CMakeLists.txt`:

```cmake
find_package(ArenaPro CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE ArenaPro::ArenaPro)
```

> vcpkg and Conan packages are built and verified (recipe in
> `packaging/recipes/arenapro/`, port in `packaging/vcpkg/ports/arenapro/`),
> but not yet published to the public registries. This section will be
> updated once they are.

## <a id="quick-start"></a>🚀 Quick Start

```cpp
#include <ArenaPro/Arena.h>

int main() {
    ArenaPro::Arena<> arena(1024);

    std::byte* raw = arena.allocate(64);         // raw bytes
    auto* widget = arena.create<Widget>(1, 2);   // constructed in place

    arena.destroy(widget); // runs ~Widget(); storage stays reserved
}
```

Scratch work scoped to a single call, rolled back automatically:

```cpp
#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>

void process(ArenaPro::Arena<>& arena) {
    ArenaPro::ArenaScope scope(arena); // opens a frame

    arena.allocate(256); // scratch space for this call only
    // ... frame rolls back automatically when scope goes out of scope,
    // even if an exception unwinds through it
}
```

Tracking usage with statistics enabled:

```cpp
ArenaPro::Arena<true> arena(4096); // EnableStats = true

arena.allocate(128);

const auto& stats = arena.getStats();
std::cout << stats.allocations_ << " allocations, "
          << stats.peakUsed_ << " bytes at peak\n";
```

## <a id="project-structure"></a>🗂️ Project Structure

```
ArenaAllocator/
├── include/
│   └── ArenaPro/
│       ├── Arena.h
│       ├── Arena.tpp
│       ├── ArenaScope.h
│       └── Contract.h
│
├── tests/
│   ├── support/
│   ├── suite/
│   ├── test_main.cpp
│   └── CMakeLists.txt
│
├── benchmarks/
│   ├── support/
│   ├── suite/
│   ├── baselines/
│   ├── bench_main.cpp
│   └── CMakeLists.txt
│
├── examples/
│   ├── support/
│   ├── suite/
│   ├── example_main.cpp
│   └── CMakeLists.txt
│
├── regression/
│   ├── support/
│   ├── regression_main.cpp
│   └── CMakeLists.txt
│
├── packaging/
│   ├── README.md
│   ├── recipes/
│   │   └── arenapro/
│   ├── vcpkg/
│   │   └── ports/
│   │       └── arenapro/
│   └── vcpkg-smoke-test/
│
├── scripts/
│   └── update_package_files.py
│
├── .github/
│   ├── releases/
│   └── workflows/
│
├── cmake/
│   └── ArenaProConfig.cmake.in
│
├── docs/
│   ├── Doxyfile
│   └── README.md
│
├── .gitignore
├── CMakeLists.txt
├── README.md
└── LICENSE
```

## <a id="development"></a>🛠️ Development

The from-source install above builds the library only. To work on
ArenaPro itself — running tests, benchmarks, or the regression tool —
build with everything enabled (the default):

```bash
cmake -B build
cmake --build build
```

**Run the test suite:**

```bash
ctest --test-dir build
```

**Run benchmarks and check for regressions:**

```bash
./build/benchmarks
./build/regression                  # latest baseline vs. benchmarks/results/benchmark_results.json
./build/regression v1.2.0           # a specific baseline vs. current
./build/regression v1.2.0 v1.4.0    # two baselines against each other
```

`regression` picks the latest baseline by semantic version (`v1.10.0`
correctly outranks `v1.9.0`), not alphabetical filename order, and
auto-names its output (`regression_v1.2.0_vs_current.md`/`.json`, etc.).

See [packaging/README.md](packaging/README.md) for notes on verifying the vcpkg
port and Conan recipe locally.

## <a id="benchmarks"></a>📊 Benchmarks

Measured against `stdArena` (a naive `new[]` + linear-scan baseline),
same build, at 10K / 100K / 1M iterations (`benchmarks/baselines/v1.0.0.json`
has the full dataset).

| Operation | ArenaPro (1M) | stdArena (1M) | Δ |
|---|---|---|---|
| `Allocate() At Capacity (Failure Path)` | 308.34 us | 1.76 s | +569727.4% |
| `Reset() + Refill` | 101.67 ms | 14.16 s | +13825.6% |
| `Allocate<T>() Large Type` | 821.41 us | 1.63 ms | +98.2% |
| `Allocate() Large` | 680.08 us | 1.27 ms | +86.9% |
| `Allocate() Over-aligned` | 770.38 us | 1.27 ms | +64.8% |
| `Create<T>() Non-trivial` | 3.03 ms | 3.84 ms | +26.6% |
| `Allocate() 4096-byte Aligned` | 1.41 ms | 1.76 ms | +24.6% |
| `Allocate() Small` | 1.11 ms | 1.30 ms | +17.2% |
| `Destroy<T>() Trivial` | 320.11 us | 308.32 us | -3.7% |
| `Allocate() 4 KiB Buffer` | 2.66 ms | 2.04 ms | -23.3% |
| `Construction` | 40.12 ms | 7.47 ms | -81.4% |

ArenaPro's fixed-buffer, bump-pointer design pays off most on the
exhaustion path (a bounds check and a `nullptr` return versus
`stdArena` actually growing), bulk churn (`Reset()` + refill), and
large or over-aligned allocations, where `stdArena`'s per-call
bookkeeping shows up directly.

The trade-off: the buffer is allocated once, eagerly and aligned, at
construction — so `Construction` is consistently slower than
`stdArena`'s lazy setup, and small buffers (`Allocate() 4 KiB Buffer`)
don't fully amortize that upfront alignment cost the way larger
buffers do.

## <a id="documentation"></a>📖 Documentation

Full API reference, generated with Doxygen from `docs/Doxyfile`:

**https://privateMwb.github.io/ArenaAllocator/**

## <a id="contributing"></a>🤝 Contributing

Issues and pull requests are welcome. Before submitting a PR:

- Run the test suite (`ctest --test-dir build`)
- If you're changing a hot path, run `./build/regression` and mention
  the results in your PR description

## <a id="changelog"></a>📝 Changelog

See the [Releases](https://github.com/privateMwb/ArenaAllocator/releases)
page for version history and release notes.

## <a id="license"></a>📄 License

MIT — see [LICENSE](LICENSE) for details.
