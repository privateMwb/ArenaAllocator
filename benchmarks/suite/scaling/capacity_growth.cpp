// Arena Scaling Benchmark Suite — Capacity Growth
// Measures how Arena allocate() cost changes as buffer capacity itself
// grows, against stdArena.
//
// Unlike the SMALL/MEDIUM/LARGE iteration tiers (same operation,
// repeated more times), this sweeps buffer size itself and repeats the
// same allocate() call at each size — isolating whether per-call cost
// depends on the total size of the underlying buffer.
//
// The Arena side wraps every call in beginFrame()/endFrame(), rolling
// the cursor back immediately after each measured allocate(). Without
// this, a large enough iteration count (the LARGE tier alone is
// 1,000,000 calls) would exhaust any buffer size small enough to be
// worth calling "small", silently turning the back half of the run
// into bounds-check failures instead of real allocations — exactly
// the failure-path behavior exhaustion.cpp measures on purpose. The
// wrapper adds a small, constant amount of frame bookkeeping to every
// call, but that constant is identical across all three sizes, so the
// relative comparison between them stays meaningful.
//
// stdArena needs no such wrapper: a default-constructed
// monotonic_buffer_resource transparently pulls another block from its
// upstream resource once its initial buffer is exhausted, rather than
// failing — that's its normal, intended behavior, not a workaround.
//
// Covers:
// - allocate() in a 4 KiB buffer
// - allocate() in a 1 MiB buffer
// - allocate() in a 64 MiB buffer

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSmallCapacity = 4 * 1024;
constexpr std::size_t kMediumCapacity = 1 * 1024 * 1024;
constexpr std::size_t kLargeCapacity = 64 * 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures allocate() cost in a 4 KiB buffer.
static void bench_capacity_small() {
    Arena<false> cSrc(kSmallCapacity);
    stdArena sSrc(kSmallCapacity);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize)); };

    BENCH("allocate() 4 KiB buffer", c, s);
}

// Measures allocate() cost in a 1 MiB buffer.
static void bench_capacity_medium() {
    Arena<false> cSrc(kMediumCapacity);
    stdArena sSrc(kMediumCapacity);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize)); };

    BENCH("allocate() 1 MiB buffer", c, s);
}

// Measures allocate() cost in a 64 MiB buffer.
static void bench_capacity_large() {
    Arena<false> cSrc(kLargeCapacity);
    stdArena sSrc(kLargeCapacity);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize)); };

    BENCH("allocate() 64 MiB buffer", c, s);
}

// Executes all capacity growth benchmark cases.
static void run_benchmarks() {
    bench_capacity_small();
    std::cout << "\n";

    bench_capacity_medium();
    std::cout << "\n";

    bench_capacity_large();
}

REGISTER_BENCH_SUITE();
