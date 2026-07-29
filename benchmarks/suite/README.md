# Benchmark Suite

This document describes the benchmark categories under `suite/` — what
each one measures, and the individual benchmarks it contains.

Every benchmark compares Arena against stdArena — a
`std::pmr::monotonic_buffer_resource`, the standard library's own
linear/bump allocator, the conventional way this kind of allocation
behavior is built in C++. A category can support more than one standard
for comparison, but for now each category is benchmarked against a
single standard.

Every `BENCH()` call, in every category below, is automatically repeated
at three iteration tiers — SMALL (10K), MEDIUM (100K), and LARGE (1M) —
to smooth out timing noise and show whether relative performance holds
steady as call volume increases. This applies uniformly across the whole
suite; it is not specific to any one category. The **Scaling** category
below measures something different: how per-operation cost changes as
capacity itself grows or shrinks, independent of iteration count.

Some benchmarks have no meaningful stdArena equivalent — a bare
`memory_resource` tracks no ownership, usage, or allocation statistics,
supports no nested checkpoints, and isn't movable. Those run through
`BENCH_SOLO()` instead of `BENCH()`, timing Arena alone.

---

## Access

Benchmarks read-only operations against an arena that is already holding
allocations — ownership checks, snapshotting what's been allocated so
far, and querying current usage.

### Benchmarks

- `ownership.cpp` — `owns()` hit, `owns()` miss (solo, no stdArena
  equivalent)
- `view_span.cpp` — `view()` over a populated arena, `view()` over an
  empty arena (solo, no stdArena equivalent)
- `state_query.cpp` — `used()`, `remaining()`, `capacity()`,
  `frameDepth()` (solo, no stdArena equivalent)

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
allocating raw bytes, allocating storage for a type, constructing in
place, destroying, and reclaiming space via reset or frame rollback.

### Benchmarks

- `allocate.cpp` — `allocate()` small, large, over-aligned (paired
  against stdArena)
- `typed_allocate.cpp` — `allocate<T>()` for a small type, a larger
  multi-member type (paired against stdArena)
- `construct.cpp` — `create<T>()` with a trivial constructor, a
  non-trivial multi-argument constructor (paired against stdArena)
- `destroy.cpp` — `destroy<T>()` with a trivial destructor, a
  non-trivial destructor (paired against stdArena)
- `reset_refill.cpp` — `reset()` alone, `reset()` then refilling to a
  fixed entry count (paired against stdArena's `release()`)
- `begin_end.cpp` — `beginFrame()`/`endFrame()` pair (solo, no stdArena
  equivalent)
- `nested_frames.cpp` — deeply nested `beginFrame()`/`endFrame()`,
  repeated push/pop (solo, no stdArena equivalent)

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction, and
moving. Arena has no copy constructor, so this category covers move
only.

### Benchmarks

- `construction.cpp` — constructing an empty arena sized for N bytes
  (paired against stdArena — Arena allocates eagerly, stdArena defers
  to first use, so this compares two genuinely different construction
  strategies, not just two names for the same operation)
- `move.cpp` — move-construct, move-assign (ping-ponged between two
  populated arenas) (solo — `monotonic_buffer_resource` is neither
  copyable nor movable)
- `scope_raii.cpp` — `ArenaScope` construction/destruction (solo, no
  stdArena equivalent)

---

## Scaling

Benchmarks how per-operation cost changes as capacity itself grows or
shrinks — a separate axis from the SMALL/MEDIUM/LARGE iteration tiers
described above: those repeat the same fixed-size operation more times,
while Scaling grows or shrinks the buffer, alignment, or remaining
headroom itself and observes the resulting cost.

### Benchmarks

- `capacity_growth.cpp` — `allocate()` across increasing buffer sizes:
  4 KiB, 1 MiB, 64 MiB (paired against stdArena)
- `alignment_scaling.cpp` — `allocate()` across increasing alignment
  requests: 4, 64, 4096 bytes (paired against stdArena)
- `exhaustion.cpp` — `allocate()` with room to spare, `allocate()` at
  capacity (failure path) (paired against a bounded stdArena, using
  `std::pmr::null_memory_resource()` as its upstream)

---

## Utility

Benchmarks bookkeeping operations that don't belong to any of the
categories above — running allocation statistics.

### Benchmarks

- `stats.cpp` — `getStats()` — total/current/peak usage, allocation
  count (solo, no stdArena equivalent)
