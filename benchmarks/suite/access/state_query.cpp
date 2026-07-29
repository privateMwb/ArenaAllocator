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

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures used().
static void bench_used() {
    Arena<false> aSrc(kSize);
    doNotOptimize(aSrc.allocate(64));

    auto a = [&] {
        std::size_t v = aSrc.used();
        doNotOptimize(v);
    };

    BENCH_SOLO("used()", a);
}

// Measures remaining().
static void bench_remaining() {
    Arena<false> aSrc(kSize);
    doNotOptimize(aSrc.allocate(64));

    auto a = [&] {
        std::size_t v = aSrc.remaining();
        doNotOptimize(v);
    };

    BENCH_SOLO("remaining()", a);
}

// Measures capacity().
static void bench_capacity() {
    Arena<false> aSrc(kSize);
    doNotOptimize(aSrc.allocate(64));

    auto a = [&] {
        std::size_t v = aSrc.capacity();
        doNotOptimize(v);
    };

    BENCH_SOLO("capacity()", a);
}

// Measures frameDepth() with nested frames open.
static void bench_frame_depth() {
    Arena<false> aSrc(kSize);
    aSrc.beginFrame();
    aSrc.beginFrame();
    aSrc.beginFrame();

    auto a = [&] {
        std::size_t v = aSrc.frameDepth();
        doNotOptimize(v);
    };

    BENCH_SOLO("frameDepth()", a);
}

// Executes all state query benchmark cases.
static void run_benchmarks() {
    bench_used();
    std::cout << "\n";

    bench_remaining();
    std::cout << "\n";

    bench_capacity();
    std::cout << "\n";

    bench_frame_depth();
}

REGISTER_BENCH_SUITE();
