// Arena Scaling Benchmark Suite — Alignment Scaling
// Measures how Arena allocate() cost changes as the requested
// alignment grows, against stdArena.
//
// The Arena side wraps every call in beginFrame()/endFrame() for the
// same reason as capacity_growth.cpp: a page-aligned request can burn
// close to a full 4 KiB of padding per call, and across the LARGE
// iteration tier (1,000,000 calls) that adds up to gigabytes — the
// wrapper rolls the cursor back after every measured call so no buffer
// size has to be sized for that, and the added bookkeeping is the same
// constant at every alignment, so the comparison across them still
// holds. stdArena needs no wrapper; its default upstream resource
// transparently grows on demand.
//
// Covers:
// - allocate() at the natural alignment of an int (4 bytes)
// - allocate() at cache-line alignment (64 bytes)
// - allocate() at page alignment (4096 bytes)

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
constexpr std::size_t kNaturalAlignment = alignof(int);
constexpr std::size_t kCacheLineAlignment = 64;
constexpr std::size_t kPageAlignment = 4096;
} // namespace

// Measures allocate() at the natural alignment of an int.
static void bench_alignment_natural() {
    Arena<false> cSrc(kCapacityBytes, kNaturalAlignment);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize, kNaturalAlignment));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize, kNaturalAlignment)); };

    BENCH("allocate() 4-byte aligned", c, s);
}

// Measures allocate() at cache-line alignment.
static void bench_alignment_cache_line() {
    Arena<false> cSrc(kCapacityBytes, kCacheLineAlignment);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize, kCacheLineAlignment));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize, kCacheLineAlignment)); };

    BENCH("allocate() 64-byte aligned", c, s);
}

// Measures allocate() at page alignment.
static void bench_alignment_page() {
    Arena<false> cSrc(kCapacityBytes, kPageAlignment);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize, kPageAlignment));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize, kPageAlignment)); };

    BENCH("allocate() 4096-byte aligned", c, s);
}

// Executes all alignment scaling benchmark cases.
static void run_benchmarks() {
    bench_alignment_natural();
    std::cout << "\n";

    bench_alignment_cache_line();
    std::cout << "\n";

    bench_alignment_page();
}

REGISTER_BENCH_SUITE();
