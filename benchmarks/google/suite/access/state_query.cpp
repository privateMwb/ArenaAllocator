// Arena Access Benchmark Suite — State Query
// Measures Arena state-query performance.
//
// std::pmr::memory_resource exposes no usage introspection at all —
// no used()/remaining()/capacity(), no frame depth — so every case
// here runs solo.
//
// Covers:
// - used(), on an arena with some allocations already made
// - remaining(), on the same arena
// - capacity(), on the same arena
// - frameDepth(), on an arena with nested frames open

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures used().
static void used(benchmark::State& state) {
    Arena<false> aSrc(kSize);
    benchmark::DoNotOptimize(aSrc.allocate(64));

    for (auto _ : state) {
        std::size_t v = aSrc.used();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(used);

// Measures remaining().
static void remaining(benchmark::State& state) {
    Arena<false> aSrc(kSize);
    benchmark::DoNotOptimize(aSrc.allocate(64));

    for (auto _ : state) {
        std::size_t v = aSrc.remaining();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(remaining);

// Measures capacity().
static void capacity(benchmark::State& state) {
    Arena<false> aSrc(kSize);
    benchmark::DoNotOptimize(aSrc.allocate(64));

    for (auto _ : state) {
        std::size_t v = aSrc.capacity();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(capacity);

// Measures frameDepth() with nested frames open.
static void frame_depth(benchmark::State& state) {
    Arena<false> aSrc(kSize);
    aSrc.beginFrame();
    aSrc.beginFrame();
    aSrc.beginFrame();

    for (auto _ : state) {
        std::size_t v = aSrc.frameDepth();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(frame_depth);
