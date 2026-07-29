# Test Suite

This document describes the test categories under `suite/` — what each
one verifies, and the individual test files it contains.

Unlike the benchmark suite, tests validate the library's own
correctness directly — there is no reference implementation to compare
against, so results are simply pass or fail.

Every test suite registers itself automatically via
`REGISTER_TEST_SUITE()` at startup, and is assigned a sequential id
within its category (e.g. `C1`, `C2` for Concurrency; `U1`, `U2` for
Unit) — there's no suite list to maintain by hand. This applies
uniformly across every category below.

---

## Concurrency

Verifies thread-safety — concurrent reads and writes from multiple
threads, and correctness under simultaneous access.

### Tests

- `external_locking_contract.cpp` — Concurrent allocate() calls and beginFrame()/allocate()/endFrame() cycles, serialized by a caller-supplied mutex, never overlap and total correctly
- `concurrent_read_only.cpp` — The const observers (owns(), used(), remaining(), capacity(), frameDepth(), getStats(), view()) stay consistent across concurrent, lock-free callers once state is settled
- `scope_per_thread.cpp` — One Arena/ArenaScope pair per thread, confirming independently-owned arenas don't leak state into one another

---

## Integration

Verifies multiple components working together end-to-end — for
example, Arena and ArenaScope combined across a realistic sequence —
rather than a single function in isolation.

### Tests

- `scope_rollback.cpp` — ArenaScope opens/closes a frame via RAII, rolls back on a thrown exception, and freed bytes are reused
- `nested_frames.cpp` — Nested frames restore the correct checkpoint at each level; rolling back an inner frame reuses its space while an outer frame's allocations are untouched
- `stats_tracking.cpp` — Stats stay correct across a mixed allocate/frame sequence; a frame rollback lowers currentUsed_ but never totalAllocated_
- `alloc_reset_reuse.cpp` — Fill to capacity, then reset(), then reuse from the start of the buffer
- `create_destroy_cycle.cpp` — destroy() alone doesn't reclaim storage; reset() does, and a later create() reuses the freed bytes

---

## Lifecycle

Verifies object lifetime operations — construction, destruction, and
moving.

### Tests

- `construction.cpp` — Ctor behavior across valid sizes and alignments; an unsatisfiable size throws std::bad_alloc
- `destruction.cpp` — Live, un-destroyed objects are not destructed when the arena itself is torn down; a moved-from arena destructs safely
- `move_semantics.cpp` — Move construction and move assignment: state transfer, moved-from validity, self-move-assignment safety

---

## Regression

Verifies that a specific, previously fixed bug — or a deliberately
assert-only contract — stays exactly as intended. One test per
resolved issue or pinned contract, added at the time it's settled.

### Tests

- `ctor_validation_order.cpp` — size validation now runs before the buffer is allocated, not after
- `frame_depth_contract.cpp` — exceeding the frame stack, or closing one when none is open, is an AP_PRE (debug-only) contract violation by design, not a runtime-checked error
- `alignment_contract.cpp` — requesting an alignment above the arena's base alignment is likewise an AP_PRE-only contract violation, not a runtime-checked error

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

- `allocate.cpp` — Returns a valid pointer within capacity, nullptr when out of space, cursor advances across calls, zero-size requests succeed
- `create.cpp` — Forwards constructor arguments, returns nullptr without constructing on a failed allocation, a throwing constructor propagates
- `destroy.cpp` — Runs the object's destructor without reclaiming its storage
- `begin_frame.cpp` — Opening a frame increments frame depth; nested frames are tracked correctly
- `end_frame.cpp` — Rewinds the cursor to the matching checkpoint, decrements frame depth, reclaimed space is reusable
- `reset.cpp` — Resets cursor and frame depth, clears stats when enabled, leaves capacity() unchanged
- `owns.cpp` — Live allocations are owned; foreign pointers, the one-past-the-end address, and nullptr are not
- `view.cpp` — Span size matches used(), span covers exactly the allocated bytes, a fresh arena has a zero-size view
- `alignment.cpp` — Default alignment matches alignof(max_align_t); a custom alignment is honored; padding consumes capacity as needed
- `observers.cpp` — used(), remaining(), capacity(), frameDepth(), and getStats() all reflect current state correctly
