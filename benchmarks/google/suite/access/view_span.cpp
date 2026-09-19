// Arena Access Benchmark Suite — View Span
// Measures Arena view() performance.
//
// std::pmr::memory_resource exposes no way to snapshot what's been
// allocated so far, so this runs solo.
//
// Covers:
// - view(), on an arena with some allocations already made
// - view(), on a freshly constructed, still-empty arena

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <span>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures view() over an arena with data already allocated.
static void ViewPopulated(benchmark::State& state) {
    Arena<false> aSrc(kSize);
    benchmark::DoNotOptimize(aSrc.allocate(256));

    for (auto _ : state) {
        std::span<const std::byte> v = aSrc.view();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(ViewPopulated);

// Measures view() over a freshly constructed, empty arena.
static void ViewEmpty(benchmark::State& state) {
    Arena<false> aSrc(kSize);

    for (auto _ : state) {
        std::span<const std::byte> v = aSrc.view();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(ViewEmpty);
