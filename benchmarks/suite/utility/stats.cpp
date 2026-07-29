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

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures getStats() on an arena with a nonzero allocation history.
static void bench_get_stats() {
    Arena<true> aSrc(kSize);
    doNotOptimize(aSrc.allocate(64));
    doNotOptimize(aSrc.allocate(128));
    doNotOptimize(aSrc.allocate(32));

    auto a = [&] {
        const auto& v = aSrc.getStats();
        doNotOptimize(v);
    };

    BENCH_SOLO("getStats()", a);
}

// Executes all stats benchmark cases.
static void run_benchmarks() {
    bench_get_stats();
}

REGISTER_BENCH_SUITE();
