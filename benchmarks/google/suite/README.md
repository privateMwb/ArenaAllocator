# Google Benchmark Suite

This document describes the benchmark categories under `suite/` — what each
one measures, and the individual benchmarks it contains. Same categories as
`../custom/suite/`, reimplemented with Google Benchmark instead of the custom
framework.

| Category | Focus |
|---|---|
| [Access](#access) | Read-only ownership checks, snapshots, and state queries on an already-populated arena |
| [Core](#core) | Allocating, constructing, destroying, resetting, and frame rollback |
| [Lifecycle](#lifecycle) | Construction, moving, and scoped frames |
| [Scaling](#scaling) | Cost vs. capacity, alignment, and exhaustion, independent of iteration count |
| [Utility](#utility) | Running allocation statistics |
| [Conventions](#conventions) | Registration, iteration-count, and macro conventions specific to Google Benchmark |

Every benchmark compares ArenaPro's `Arena` against `stdArena` — a
`std::pmr::monotonic_buffer_resource`, the standard library's own linear/bump
allocator, and the conventional way this kind of allocation behavior is built
in C++. A category can support more than one standard for comparison, but for
now each category is benchmarked against a single standard.

Each comparison is registered as two benchmarks, one per side, suffixed
`Arena` or `Std` — for example `AllocateSmallArena` and `AllocateSmallStd` —
so the two sit next to each other in the output. The tables below write each
pair as one name.

Iteration counts are chosen by Google Benchmark, except where a benchmark's
state accumulates across iterations and a fixed count is set instead (see
[Conventions](#conventions)). The **Scaling** category below measures
something different: how per-operation cost changes as capacity itself
grows or shrinks, independent of iteration count.

Some benchmarks have no meaningful stdArena equivalent — a bare
`memory_resource` tracks no ownership, usage, or allocation statistics,
supports no nested checkpoints, and isn't movable. Those are registered as a
single benchmark with no suffix, timing Arena alone.

---

## Access

Benchmarks read-only operations against an arena that is already holding
allocations — ownership checks, snapshotting what's been allocated so far,
and querying current usage.

### Benchmarks

| File | What it covers |
|---|---|
| `ownership.cpp` | `owns()` hit (`OwnsHit`) and `owns()` miss (`OwnsMiss`) (solo, no stdArena equivalent) |
| `view_span.cpp` | `view()` over a populated arena (`ViewPopulated`), and over an empty arena (`ViewEmpty`) (solo, no stdArena equivalent) |
| `state_query.cpp` | `used()` (`Used`), `remaining()` (`Remaining`), `capacity()` (`Capacity`), and `frameDepth()` (`FrameDepth`) (solo, no stdArena equivalent) |

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
allocating raw bytes, allocating storage for a type, constructing in place,
destroying, and reclaiming space via reset or frame rollback.

### Benchmarks

| File | What it covers |
|---|---|
| `allocate.cpp` | `allocate()` small (`AllocateSmall`), large (`AllocateLarge`), and over-aligned (`AllocateAligned`) |
| `type_allocate.cpp` | `allocate<T>()` for a small type (`AllocateSmallType`), and for a larger multi-member type (`AllocateLargeType`) |
| `construct.cpp` | `create<T>()` with a trivial constructor (`ConstructTrivial`), and with a non-trivial multi-argument constructor (`ConstructNonTrivial`) |
| `destroy.cpp` | `destroy<T>()` with a trivial destructor (`DestroyTrivial`), and with a non-trivial destructor (`DestroyNonTrivial`) |
| `reset_refill.cpp` | `reset()` alone (`Reset`), and `reset()` then refilling to a fixed entry count (`ResetRefill`), against stdArena's `release()` |
| `begin_end.cpp` | `beginFrame()`/`endFrame()` pair (solo, no stdArena equivalent) |
| `nested_frames.cpp` | Deeply nested `beginFrame()`/`endFrame()`, repeated push/pop (solo, no stdArena equivalent) |

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction, and
moving. Arena has no copy constructor, so this category covers move only.

### Benchmarks

| File | What it covers |
|---|---|
| `construction.cpp` | Constructing an empty arena sized for N bytes (`Construction`) — Arena allocates eagerly, stdArena defers to first use, so this compares two genuinely different construction strategies |
| `move.cpp` | Move construction (`MoveConstruct`), and move assignment ping-ponged between two populated arenas (`MoveAssign`) (solo — `monotonic_buffer_resource` is neither copyable nor movable) |
| `scope_raii.cpp` | `ArenaScope` construction/destruction (`ScopeRaii`) (solo, no stdArena equivalent) |

---

## Scaling

Benchmarks how per-operation cost changes as capacity itself grows or
shrinks — a separate axis from iteration count: iterations repeat the same
fixed-size operation more times, while Scaling grows or shrinks the buffer,
alignment, or remaining headroom itself and observes the resulting cost.

### Benchmarks

| File | What it covers |
|---|---|
| `capacity_growth.cpp` | `allocate()` across increasing buffer sizes: 4 KiB (`CapacitySmall`), 1 MiB (`CapacityMedium`), and 64 MiB (`CapacityLarge`) |
| `alignment_scaling.cpp` | `allocate()` across increasing alignment requests: 4 (`AlignmentNatural`), 64 (`AlignmentCacheLine`), and 4096 bytes (`AlignmentPage`) |
| `exhaustion.cpp` | `allocate()` with room to spare (`AllocateSuccess`), and `allocate()` at capacity, the failure path (`AllocateFailure`), against a bounded stdArena using `std::pmr::null_memory_resource()` as its upstream |

---

## Utility

Benchmarks bookkeeping operations that don't belong to any of the categories
above — running allocation statistics.

### Benchmarks

| File | What it covers |
|---|---|
| `stats.cpp` | `getStats()` (`GetStats`) — total/current/peak usage and allocation count (solo, no stdArena equivalent) |


---

## Conventions

- **Registration** — each benchmark is a
  `static void Name(benchmark::State& state)` followed by `BENCHMARK(Name);`,
  with no `BM_` prefix. There is no `BENCHMARK_MAIN()`; the suite supplies
  its own `main`.
- **Includes** — every file includes `<support/framework.h>`, and paired
  suites also include `<support/reference.h>` for `stdArena`, followed by
  `<benchmark/benchmark.h>`.
- **Preventing elision** — `benchmark::DoNotOptimize()` takes each loop
  body's result so the call can't be optimized away. A loop body with no
  result (`reset()`, `destroy()`) has nothing to pass.
- **Fixed iteration counts** — Google Benchmark picks its own iteration
  count, which would exhaust a bump-allocated buffer. Benchmarks whose state
  accumulates (allocation, construction, `destroy()`) set
  `->Iterations(kIterations)`, the same on both sides of a pair. Where the
  buffer can't be sized for that count (page-aligned requests, sweeping
  buffer size), each Arena call is wrapped in `beginFrame()` / `endFrame()`
  to roll the cursor back, and stdArena is left unbounded. `destroy.cpp`
  builds a pool of exactly `kIterations` objects, so its `->Iterations()`
  must not be removed — an auto-tuned count would index past the end of the
  pool. Benchmarks that never use up capacity stay auto-tuned.
- **Setup outside the loop** — only the `for (auto _ : state)` body is timed.
  The exception is an operation that consumes its input: `MoveConstruct`
  rebuilds its source inside the loop.
