// Arena Core Benchmark Suite — Typed Allocate
// Measures Arena allocate<T>() performance against stdArena.
//
// Each case owns a buffer sized generously above the LARGE iteration
// tier, so repeated calls never exhaust capacity mid-benchmark — only
// the steady-state bump-allocation path is measured.
//
// Covers:
// - allocate<T>() for a small, trivially-typed T
// - allocate<T>() for a larger, multi-member T

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;

struct Small {
    int value;
};

struct Large {
    double a, b, c, d;
    int e, f;
};
} // namespace

// Measures allocate<T>() for a small, single-member type.
static void bench_allocate_small_type() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        Small* p = cSrc.allocate<Small>();
        doNotOptimize(p);
    };

    auto s = [&] {
        void* p = sSrc.allocate(sizeof(Small), alignof(Small));
        doNotOptimize(p);
    };

    BENCH("allocate<T>() small type", c, s);
}

// Measures allocate<T>() for a larger, multi-member type.
static void bench_allocate_large_type() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        Large* p = cSrc.allocate<Large>();
        doNotOptimize(p);
    };

    auto s = [&] {
        void* p = sSrc.allocate(sizeof(Large), alignof(Large));
        doNotOptimize(p);
    };

    BENCH("allocate<T>() large type", c, s);
}

// Executes all typed allocate benchmark cases.
static void run_benchmarks() {
    bench_allocate_small_type();
    std::cout << "\n";

    bench_allocate_large_type();
}

REGISTER_BENCH_SUITE();
