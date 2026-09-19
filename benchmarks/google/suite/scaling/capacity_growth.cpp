// Arena Scaling Benchmark Suite — Capacity Growth
// Measures how Arena allocate() cost changes as buffer capacity itself
// grows, against stdArena.
//
// Unlike simply repeating the same operation more times, this sweeps
// buffer size itself and repeats the same allocate() call at each size
// — isolating whether per-call cost depends on the total size of the
// underlying buffer.
//
// The Arena side wraps every call in beginFrame()/endFrame(), rolling
// the cursor back immediately after each measured allocate(). Without
// this, a large enough iteration count (kIterations alone is 1,000,000
// calls) would exhaust any buffer size small enough to be worth calling
// "small", silently turning the back half of the run into bounds-check
// failures instead of real allocations — exactly the failure-path
// behavior exhaustion.cpp measures on purpose. The wrapper adds a
// small, constant amount of frame bookkeeping to every call, but that
// constant is identical across all three sizes, so the relative
// comparison between them stays meaningful.
//
// stdArena needs no such wrapper: a default-constructed
// monotonic_buffer_resource transparently pulls another block from its
// upstream resource once its initial buffer is exhausted, rather than
// failing — that's its normal, intended behavior, not a workaround.
//
// Every case runs a fixed kIterations, on both sides, so the two are
// compared over identical work and stdArena's upstream growth stays
// bounded. Google Benchmark's auto-tuned count would keep growing it
// for as long as the timer ran.
//
// Covers:
// - allocate() in a 4 KiB buffer
// - allocate() in a 1 MiB buffer
// - allocate() in a 64 MiB buffer

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSmallCapacity = 4 * 1024;
constexpr std::size_t kMediumCapacity = 1 * 1024 * 1024;
constexpr std::size_t kLargeCapacity = 64 * 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
constexpr std::size_t kIterations = 1'000'000;
} // namespace

// Measures Arena allocate() cost in a 4 KiB buffer.
static void CapacitySmallArena(benchmark::State& state) {
    Arena<false> cSrc(kSmallCapacity);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    }
}
BENCHMARK(CapacitySmallArena)->Iterations(kIterations);

// Measures stdArena allocate() cost in a 4 KiB buffer.
static void CapacitySmallStd(benchmark::State& state) {
    stdArena sSrc(kSmallCapacity);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
}
BENCHMARK(CapacitySmallStd)->Iterations(kIterations);

// Measures Arena allocate() cost in a 1 MiB buffer.
static void CapacityMediumArena(benchmark::State& state) {
    Arena<false> cSrc(kMediumCapacity);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    }
}
BENCHMARK(CapacityMediumArena)->Iterations(kIterations);

// Measures stdArena allocate() cost in a 1 MiB buffer.
static void CapacityMediumStd(benchmark::State& state) {
    stdArena sSrc(kMediumCapacity);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
}
BENCHMARK(CapacityMediumStd)->Iterations(kIterations);

// Measures Arena allocate() cost in a 64 MiB buffer.
static void CapacityLargeArena(benchmark::State& state) {
    Arena<false> cSrc(kLargeCapacity);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    }
}
BENCHMARK(CapacityLargeArena)->Iterations(kIterations);

// Measures stdArena allocate() cost in a 64 MiB buffer.
static void CapacityLargeStd(benchmark::State& state) {
    stdArena sSrc(kLargeCapacity);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
}
BENCHMARK(CapacityLargeStd)->Iterations(kIterations);
