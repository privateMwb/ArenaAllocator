// Arena Utility Benchmark Suite — Stats
// Measures Arena getStats() performance.
//
// std::pmr::memory_resource tracks no allocation statistics at all —
// no totals, no peak usage, no allocation count — so this runs solo.
// Also only meaningful with EnableStats = true; getStats() isn't even
// callable otherwise.
//
// Covers:
// - getStats(), on an arena that has made several allocations

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures getStats() on an arena with a nonzero allocation history.
static void get_stats(benchmark::State& state) {
    Arena<true> aSrc(kSize);
    benchmark::DoNotOptimize(aSrc.allocate(64));
    benchmark::DoNotOptimize(aSrc.allocate(128));
    benchmark::DoNotOptimize(aSrc.allocate(32));

    for (auto _ : state) {
        const auto& v = aSrc.getStats();
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(get_stats);
