# Example Suite

This document describes the example categories under `suite/` — what
each one demonstrates, and the individual example files it contains.

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse
where instructive (see Misuse), so the reader sees both the correct
pattern and the mistake it guards against.

Every example file ends with `REGISTER_EXAMPLE_SUITE()`, which derives
the suite's category from its containing directory and assigns it a
sequential id within that category. This applies uniformly across
every category below.

---

## Advanced

Demonstrates deeper mechanics of the library — move semantics,
exception safety, nested frame checkpoints, and the optional
allocation-statistics tracking.

### Examples

- `move_semantics.cpp` — move construction/assignment, and what's actually safe to call on a moved-from arena
- `exception_safety.cpp` — create<T>() when a constructor throws; the arena's cursor and bookkeeping are unaffected
- `nested_frames.cpp` — opening and closing several beginFrame()/endFrame() checkpoints in sequence
- `stats_tracking.cpp` — Arena<true>'s totalAllocated_/currentUsed_/peakUsed_/allocations_, and peakUsed_ surviving a rollback

---

## Integration

Demonstrates interoperability with the rest of a codebase — embedding
the arena inside a larger class, constructing non-trivial types, and
exporting a read-only view of raw bytes.

### Examples

- `embedding_in_class.cpp` — wrapping Arena as a private implementation detail behind a domain-specific API
- `custom_types.cpp` — forwarding constructor arguments through create<T>() for a multi-member type
- `buffer_view_export.cpp` — view() as a read-only span over everything allocated so far

---

## Misuse

Demonstrates common mistakes and the undefined behavior or contract
violations they lead to, alongside the correct pattern — including
examples shown but not executed, so the reader can see what to avoid
without the program actually invoking undefined behavior.

### Examples

- `frame_depth_overflow.cpp` — exceeding the fixed 8-frame nesting limit (shown, not executed)
- `out_of_space.cpp` — allocate() returning nullptr instead of throwing when capacity runs out
- `dangling_after_reset.cpp` — a pointer left dangling by an endFrame() rollback
- `destroy_unowned_ptr.cpp` — destroy()'s owns(ptr) precondition, and the failure mode if it's violated

---

## Patterns

Demonstrates common usage idioms built on top of the core API —
scoped temporary allocation, stats-driven capacity sizing, one arena
per thread, and bulk object creation.

### Examples

- `scoped_temp_alloc.cpp` — ArenaScope for short-lived working memory inside a function
- `stats_driven_sizing.cpp` — using peakUsed_ from a representative workload to size a production arena
- `arena_per_thread.cpp` — one arena per thread instead of synchronizing access to a shared one
- `bulk_struct_alloc.cpp` — create<T>() in a loop until the arena runs out, and detecting why it stopped

---

## Quickstart

Demonstrates fundamental, everyday usage — construction, raw and
typed allocation, object lifetime, and frame-scoped rollback.

### Examples

- `basic_usage.cpp` — construction, allocate(), allocate<T>(), create<T>(), destroy(), capacity/used/remaining, reset()
- `create_and_destroy.cpp` — object lifetime across several create<T>() calls, and why destroy() doesn't shrink used()
- `frame_scope.cpp` — beginFrame()/endFrame() vs ArenaScope, and frameDepth()
