// Arena Core Benchmark Suite — Reset / Refill
// Measures Arena reset() performance against stdArena's release().
//
// Two related but distinct costs:
// - the bare reset()/release() call, in isolation
// - a full "empty a full arena, then refill it" cycle, the pattern a
//   per-frame or per-request arena is actually used in
//
// Covers:
// - reset()/release() alone, called repeatedly (idempotent — safe to
//   call on an already-empty arena, so no rebuild is needed between
//   calls)
// - reset() then refilling to kCycleSize entries, per call

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 64 * 1024 * 1024;
constexpr std::size_t kCycleSize = 100;
constexpr std::size_t kEntrySize = sizeof(int);
} // namespace

// Measures the bare reset()/release() call in isolation.
static void bench_reset() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kCycleSize; ++i) {
        doNotOptimize(cSrc.allocate(kEntrySize));
        doNotOptimize(sSrc.allocate(kEntrySize));
    }

    auto c = [&] { cSrc.reset(); };
    auto s = [&] { sSrc.release(); };

    BENCH("reset()", c, s);
}

// Measures a full reset-then-refill cycle: empty a full arena, then
// refill it back to kCycleSize entries.
static void bench_reset_refill() {
    Arena<false> cSrc(kCapacityBytes);
    stdArena sSrc(kCapacityBytes);

    for (std::size_t i = 0; i < kCycleSize; ++i) {
        doNotOptimize(cSrc.allocate(kEntrySize));
        doNotOptimize(sSrc.allocate(kEntrySize));
    }

    auto c = [&] {
        cSrc.reset();
        for (std::size_t i = 0; i < kCycleSize; ++i)
            doNotOptimize(cSrc.allocate(kEntrySize));
    };

    auto s = [&] {
        sSrc.release();
        for (std::size_t i = 0; i < kCycleSize; ++i)
            doNotOptimize(sSrc.allocate(kEntrySize));
    };

    BENCH("reset() + refill", c, s);
}

// Executes all reset/refill benchmark cases.
static void run_benchmarks() {
    bench_reset();
    std::cout << "\n";

    bench_reset_refill();
}

REGISTER_BENCH_SUITE();
