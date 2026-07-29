// Arena Core Benchmark Suite — Allocate
// Measures Arena allocate() performance against stdArena,
// the standard library's own linear/bump allocator.
//
// Each case owns a buffer sized generously above the LARGE iteration
// tier, so repeated calls never exhaust capacity mid-benchmark — only
// the steady-state bump-allocation path is measured, with no branching
// to the failure path.
//
// Covers:
// - allocate() of a small, word-sized block at the default alignment
// - allocate() of a larger, cache-line-sized block at the default alignment
// - allocate() of a small block at an over-aligned boundary (64 bytes)

#include <support/framework.h>

#include <memory_resource>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kSmallSize = sizeof(int);
constexpr std::size_t kLargeSize = 256;
constexpr std::size_t kOverAlignment = 64;
} // namespace

// Measures allocate() of a small, word-sized block at the default
// alignment — the cheapest possible call through the hot path.
static void bench_allocate_small() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        std::byte* p = cSrc.allocate(kSmallSize);
        doNotOptimize(p);
    };

    auto s = [&] {
        void* p = sSrc.allocate(kSmallSize);
        doNotOptimize(p);
    };

    BENCH("allocate() small", c, s);
}

// Measures allocate() of a larger, cache-line-sized block at the
// default alignment.
static void bench_allocate_large() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        std::byte* p = cSrc.allocate(kLargeSize);
        doNotOptimize(p);
    };

    auto s = [&] {
        void* p = sSrc.allocate(kLargeSize);
        doNotOptimize(p);
    };

    BENCH("allocate() large", c, s);
}

// Measures allocate() of a small block at an over-aligned (64-byte)
// boundary — exercises the alignment padding path on every call.
static void bench_allocate_aligned() {
    Arena<false> cSrc(kCapacityBytes, kOverAlignment);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        std::byte* p = cSrc.allocate(kSmallSize, kOverAlignment);
        doNotOptimize(p);
    };

    auto s = [&] {
        void* p = sSrc.allocate(kSmallSize, kOverAlignment);
        doNotOptimize(p);
    };

    BENCH("allocate() over-aligned", c, s);
}

// Executes all allocate benchmark cases.
static void run_benchmarks() {
    bench_allocate_small();
    std::cout << "\n";

    bench_allocate_large();
    std::cout << "\n";

    bench_allocate_aligned();
}

REGISTER_BENCH_SUITE();
