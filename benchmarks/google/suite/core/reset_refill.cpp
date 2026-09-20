// Arena Core Benchmark Suite — Reset / Refill
// Measures Arena reset() performance against stdArena's release().
//
// Two related but distinct costs:
// - the bare reset()/release() call, in isolation
// - a full "empty a full arena, then refill it" cycle, the pattern a
//   per-frame or per-request arena is actually used in
//
// Neither case can exhaust the arena however many iterations
// Google Benchmark chooses, so iteration counts are left auto-tuned.
//
// Covers:
// - reset()/release() alone, called repeatedly (idempotent — safe to
//   call on an already-empty arena, so no rebuild is needed between
//   calls)
// - reset() then refilling to kCycleSize entries, per iteration

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kCycleSize = 100;
constexpr std::size_t kEntrySize = sizeof(int);
} // namespace

// Measures the bare Arena reset() call in isolation.
static void reset_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kCycleSize; ++i)
        benchmark::DoNotOptimize(cSrc.allocate(kEntrySize));

    for (auto _ : state)
        cSrc.reset();
}
BENCHMARK(reset_arena);

// Measures the bare stdArena release() call in isolation.
static void reset_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kCycleSize; ++i)
        benchmark::DoNotOptimize(sSrc.allocate(kEntrySize));

    for (auto _ : state)
        sSrc.release();
}
BENCHMARK(reset_std);

// Measures a full Arena reset-then-refill cycle: empty a full arena,
// then refill it back to kCycleSize entries.
static void reset_refill_arena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kCycleSize; ++i)
        benchmark::DoNotOptimize(cSrc.allocate(kEntrySize));

    for (auto _ : state) {
        cSrc.reset();
        for (std::size_t i = 0; i < kCycleSize; ++i)
            benchmark::DoNotOptimize(cSrc.allocate(kEntrySize));
    }
}
BENCHMARK(reset_refill_arena);

// Measures a full stdArena release-then-refill cycle.
static void reset_refill_std(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kCycleSize; ++i)
        benchmark::DoNotOptimize(sSrc.allocate(kEntrySize));

    for (auto _ : state) {
        sSrc.release();
        for (std::size_t i = 0; i < kCycleSize; ++i)
            benchmark::DoNotOptimize(sSrc.allocate(kEntrySize));
    }
}
BENCHMARK(reset_refill_std);
