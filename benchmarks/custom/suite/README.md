# Benchmark Suite

This document describes the benchmark categories under `suite/` — what each
one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Access](#access) | Read-only ownership checks, snapshots, and state queries on an already-populated arena |
| [Core](#core) | Allocating, constructing, destroying, resetting, and frame rollback |
| [Lifecycle](#lifecycle) | Construction, moving, and scoped frames |
| [Scaling](#scaling) | Cost vs. capacity, alignment, and exhaustion, independent of iteration count |
| [Utility](#utility) | Running allocation statistics |
| [Conventions](#conventions) | Registration, sizing, and elision conventions specific to the custom framework |

Every benchmark compares ArenaPro's `Arena` against `stdArena` — a
`std::pmr::monotonic_buffer_resource`, the standard library's own linear/bump
allocator, and the conventional way this kind of allocation behavior is built
in C++. A category can support more than one standard for comparison, but for
now each category is benchmarked against a single standard.

Every `BENCH()` call, in every category below, is automatically repeated at
three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) — to
smooth out timing noise and show whether relative performance holds steady
as call volume increases. This applies uniformly across the whole suite; it
is not specific to any one category. The **Scaling** category below measures
something different: how per-operation cost changes as capacity itself
grows or shrinks, independent of iteration count.

Some benchmarks have no meaningful stdArena equivalent — a bare
`memory_resource` tracks no ownership, usage, or allocation statistics,
supports no nested checkpoints, and isn't movable. Those run through
`BENCH_SOLO()` instead of `BENCH()`, timing Arena alone.

---

## Access

Benchmarks read-only operations against an arena that is already holding
allocations — ownership checks, snapshotting what's been allocated so far,
and querying current usage.

### Benchmarks

| File | What it covers |
|---|---|
| `ownership.cpp` | `owns()` hit and `owns()` miss (solo, no stdArena equivalent) |
| `view_span.cpp` | `view()` over a populated arena, and `view()` over an empty arena (solo, no stdArena equivalent) |
| `state_query.cpp` | `used()`, `remaining()`, `capacity()`, and `frameDepth()` (solo, no stdArena equivalent) |

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
allocating raw bytes, allocating storage for a type, constructing in place,
destroying, and reclaiming space via reset or frame rollback.

### Benchmarks

| File | What it covers |
|---|---|
| `allocate.cpp` | `allocate()` small, large, and over-aligned |
| `typed_allocate.cpp` | `allocate<T>()` for a small type, and for a larger multi-member type |
| `construct.cpp` | `create<T>()` with a trivial constructor, and with a non-trivial multi-argument constructor |
| `destroy.cpp` | `destroy<T>()` with a trivial destructor, and with a non-trivial destructor |
| `reset_refill.cpp` | `reset()` alone, and `reset()` then refilling to a fixed entry count, against stdArena's `release()` |
| `begin_end.cpp` | `beginFrame()`/`endFrame()` pair (solo, no stdArena equivalent) |
| `nested_frames.cpp` | Deeply nested `beginFrame()`/`endFrame()`, repeated push/pop (solo, no stdArena equivalent) |

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction, and
moving. Arena has no copy constructor, so this category covers move only.

### Benchmarks

| File | What it covers |
|---|---|
| `construction.cpp` | Constructing an empty arena sized for N bytes — Arena allocates eagerly, stdArena defers to first use, so this compares two genuinely different construction strategies |
| `move.cpp` | Move construction, and move assignment ping-ponged between two populated arenas (solo — `monotonic_buffer_resource` is neither copyable nor movable) |
| `scope_raii.cpp` | `ArenaScope` construction/destruction (solo, no stdArena equivalent) |

---

## Scaling

Benchmarks how per-operation cost changes as capacity itself grows or
shrinks — a separate axis from the SMALL/MEDIUM/LARGE iteration tiers
described above: those repeat the same fixed-size operation more times,
while Scaling grows or shrinks the buffer, alignment, or remaining headroom
itself and observes the resulting cost.

### Benchmarks

| File | What it covers |
|---|---|
| `capacity_growth.cpp` | `allocate()` across increasing buffer sizes: 4 KiB, 1 MiB, and 64 MiB |
| `alignment_scaling.cpp` | `allocate()` across increasing alignment requests: 4, 64, and 4096 bytes |
| `exhaustion.cpp` | `allocate()` with room to spare, and `allocate()` at capacity (failure path), against a bounded stdArena using `std::pmr::null_memory_resource()` as its upstream |

---

## Utility

Benchmarks bookkeeping operations that don't belong to any of the categories
above — running allocation statistics.

### Benchmarks

| File | What it covers |
|---|---|
| `stats.cpp` | `getStats()` — total/current/peak usage and allocation count (solo, no stdArena equivalent) |


---

## Conventions

- **Registration** — every case is a `static void bench_<name>()` called from
  the file's `run_benchmarks()`, and the file ends with
  `REGISTER_BENCH_SUITE();`. `BENCH("label", c, s)` runs Arena (`c`) against
  stdArena (`s`); `BENCH_SOLO("label", a)` times Arena alone and is used only
  when stdArena has no equivalent.
- **Preventing elision** — every lambda passes its result to
  `doNotOptimize()` so the call can't be optimized away. A lambda with no
  result (`reset()`, `destroy()`) has nothing to pass.
- **Matching work** — the stdArena side must do the same work as the Arena
  side: `create<T>()` pairs with `allocate()` plus placement-new
  (`::new (raw) T(args...)`), `destroy<T>()` with a direct destructor call,
  and `reset()` with `release()`.
- **Buffer sizing** — every `BENCH()` repeats at the SMALL, MEDIUM, and LARGE
  tiers, so the buffer is sized generously above the LARGE tier and never
  exhausts mid-run. Where that isn't practical (page-aligned requests,
  sweeping buffer size), each Arena call is wrapped in `beginFrame()` /
  `endFrame()` to roll the cursor back, and stdArena is left unbounded so it
  grows on demand.
- **Consumed inputs** — setup stays outside the lambda, except for operations
  that consume their input. `move.cpp` rebuilds its source inside the
  lambda, and `destroy.cpp` pre-builds a pool of distinct objects and
  destroys one per call, since destroying an object twice is undefined
  behavior.
