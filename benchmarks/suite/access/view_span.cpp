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

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures view() over an arena with data already allocated.
static void bench_view_populated() {
    Arena<false> aSrc(kSize);
    doNotOptimize(aSrc.allocate(256));

    auto a = [&] {
        std::span<const std::byte> v = aSrc.view();
        doNotOptimize(v);
    };

    BENCH_SOLO("view() populated", a);
}

// Measures view() over a freshly constructed, empty arena.
static void bench_view_empty() {
    Arena<false> aSrc(kSize);

    auto a = [&] {
        std::span<const std::byte> v = aSrc.view();
        doNotOptimize(v);
    };

    BENCH_SOLO("view() empty", a);
}

// Executes all view span benchmark cases.
static void run_benchmarks() {
    bench_view_populated();
    std::cout << "\n";

    bench_view_empty();
}

REGISTER_BENCH_SUITE();
