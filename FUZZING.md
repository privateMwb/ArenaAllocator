# Fuzzing

ArenaAllocator is fuzzed via [ClusterFuzzLite](https://google.github.io/clusterfuzzlite/),
running on every pull request that touches the fuzzed files, plus a
longer scheduled batch run every night.

## What's covered

**`fuzz_arena.cpp`** is a differential fuzzer: it drives an
`ArenaPro::Arena` through a random sequence of operations while an
independent shadow model tracks what the allocator's state *must* be,
comparing the two after every single operation (not just at the end),
so a failing input localizes to the exact operation that broke an
invariant.

The model deliberately re-derives the bump-pointer math with plain
modulo arithmetic rather than Arena's shift/mask formulation
(`alignForward()`/`toShift()`), so the two can only agree if Arena's
optimized hot path is actually correct. Every allocation is checked
for the **exact address** it must land on, not merely "somewhere
valid". Each returned block is then written to in full, so
AddressSanitizer sees any out-of-bounds handout, and live blocks are
re-verified against their fill pattern at every frame rollback to prove
nothing scribbled over storage that should have survived.

Specifically exercised:

- **The success/failure decision at the capacity boundary.** Request
  sizes are chosen to straddle the exact-fit point (`room - 1`,
  `room`, `room + 1`), plus zero-size requests and sizes within a few
  bytes of `SIZE_MAX` to hunt for overflow in the `aligned + size`
  computation, which Arena avoids by validating `aligned <= cap_`
  first. Alignment padding alone pushing the cursor past the end of
  the buffer is covered too.
- **Alignment correctness** for every power of two up to the arena's
  base alignment, through the raw `allocate()`, the typed
  `allocate<T>()` (including an over-aligned `alignas(32)` type), and
  `create<T>()`. Base alignments from 1 up to 64 and capacities from 1
  byte up to 4 KiB are drawn from the input, biased small.
- **`create<T>()` / `destroy()` lifecycle.** A `Tracked` type with a
  global live-instance counter proves constructors and destructors run
  exactly once, arguments are perfectly forwarded (multi-argument
  constructor), nothing is constructed when the allocation fails, and
  `destroy()` never returns storage to the arena. The counter must
  return to zero at the end of every run.
- **Frames and `ArenaScope`.** `beginFrame()`/`endFrame()` and the RAII
  guard must rewind the cursor to exactly where the frame opened,
  hand the same addresses out again, and leave everything allocated
  *before* the frame untouched. Nesting is driven up to the maximum
  frame depth.
- **`reset()`**, including that it clears the statistics.
- **Move construction, move assignment and self-move-assignment.** The
  destination must inherit the full state (buffer, cursor, frame
  stack, statistics), and the moved-from arena must be left valid and
  empty.
- **Statistics** (total / current / peak / allocation count) compared
  against the model, including that a frame rollback lowers
  `currentUsed_` but never `peakUsed_`.

Each input is run against both `Arena<true>` and `Arena<false>`, so the
`EnableStats == false` instantiation, where every statistics update
compiles away, is checked to behave identically apart from the
statistics themselves.

Built and run under both AddressSanitizer and UndefinedBehaviorSanitizer.
`build.sh` passes `-UNDEBUG`, so the `AP_PRE`/`AP_POST`/`AP_INVARIANT`
contract macros are live in the fuzz build. The harness only ever calls
Arena within its documented preconditions, so any contract abort it
reports is either a real bug or a harness bug, never expected noise.

## What's deliberately NOT covered yet

- **Precondition violations.** Passing a zero size to the constructor,
  a non-power-of-two alignment, or closing a frame that was never
  opened is undefined by contract, and the harness stays inside the
  contract on purpose.
- **Constructor failure.** `bad_alloc` from the initial buffer
  allocation is not simulated.
- **A throwing `T` constructor inside `create()`.** This harness's
  `Tracked` never throws, so it doesn't exercise the documented
  "reserved storage is simply never reused" path. A throwing element
  type (throws on the Nth construction) is what a follow-up harness
  would need, and it is a different design from differential
  comparison against a model.
- **Concurrency.** Arena is not thread-safe, so there is nothing to
  fuzz there.

## Running locally

```bash
git clone --recursive https://github.com/google/oss-fuzz.git
cd oss-fuzz
python infra/helper.py build_fuzzers --sanitizer address ArenaAllocator /path/to/ArenaAllocator
python infra/helper.py run_fuzzer ArenaAllocator fuzz_arena
```

Or, without OSS-Fuzz's tooling, directly with clang:

```bash
clang++ -std=c++20 -UNDEBUG -fsanitize=fuzzer,address \
  -Iinclude \
  fuzz/fuzz_arena.cpp \
  -o fuzz_arena

./fuzz_arena
```

Add `-fsanitize=fuzzer,undefined` instead to run under UBSan.

## Reproducing a crash

ClusterFuzzLite uploads the failing input as a workflow artifact when
a run fails. Download it, then:

```bash
./fuzz_arena path/to/crash-<hash>
```

This replays that exact byte sequence through
`LLVMFuzzerTestOneInput()` once, deterministically — no sanitizer flags
needed beyond however the binary was already built.

## Adding a new harness

1. Add `fuzz/fuzz_<target>.cpp` with an `extern "C" int
   LLVMFuzzerTestOneInput(const uint8_t*, size_t)` entry point.
2. Add the matching compile + link block to `.clusterfuzzlite/build.sh`.
3. No workflow changes needed — `cflite_pr.yml`/`cflite_batch.yml`
   build and run every binary `build.sh` produces in `$OUT`.
