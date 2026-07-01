// Arena reset benchmarks.
//
// Measures repeated full allocation cycles using reset() against
// equivalent heap allocation/deallocation patterns.
//
// Coverage:
// - Allocation to exhaustion followed by reset vs heap equivalent
// - reset() cost comparison with stats enabled vs disabled

#include "bench_helper.h"

using namespace AllocatorPro;

// Measures repeated full allocation cycles using reset()
// against an equivalent heap allocation pattern.
static void bench_exhaustion_reset() {
    constexpr std::size_t blockSize  = 64;
    constexpr std::size_t blockCount = 16;
    constexpr std::size_t poolSize   = blockSize * blockCount;

    Arena<> a{poolSize};

    auto arena_exhaustion = [&] {
        for (std::size_t i = 0; i < blockCount; ++i) {
            std::byte* ptr = a.allocate(blockSize);
            doNotOptimize(ptr);
        }
        a.reset();
        doNotOptimize();
    };
    BENCH("arena_exhaustion_reset", LARGE, arena_exhaustion);

    auto heap_exhaustion = [&] {
        void* ptrs[blockCount];

        for (std::size_t i = 0; i < blockCount; ++i) {
            ptrs[i] = ::operator new(blockSize);
            doNotOptimize(ptrs[i]);
        }

        for (std::size_t i = 0; i < blockCount; ++i) {
            ::operator delete(ptrs[i]);
        }

        doNotOptimize();
    };
    BENCH("heap_exhaustion_reset", LARGE, heap_exhaustion);
}

// Measures reset() overhead when statistics tracking is enabled
// compared to the same behavior with stats disabled.
static void bench_reset_with_stats() {
    constexpr std::size_t blockSize  = 64;
    constexpr std::size_t blockCount = 16;
    constexpr std::size_t poolSize   = blockSize * blockCount;

    Arena<false> noStats{poolSize};
    Arena<true>  withStats{poolSize};

    auto no_stats = [&] {
        for (std::size_t i = 0; i < blockCount; ++i) {
            std::byte* ptr = noStats.allocate(blockSize);
            doNotOptimize(ptr);
        }
        noStats.reset();
        doNotOptimize();
    };
    BENCH("reset_stats_disabled", LARGE, no_stats);

    auto with_stats = [&] {
        for (std::size_t i = 0; i < blockCount; ++i) {
            std::byte* ptr = withStats.allocate(blockSize);
            doNotOptimize(ptr);
        }
        withStats.reset();
        doNotOptimize();
    };
    BENCH("reset_stats_enabled", LARGE, with_stats);
}

// Executes all reset benchmark cases.
void run_reset_benchmarks() {
    setHeader("Reset Benchmarks");

    bench_exhaustion_reset();
    std::cout << "\n";

    bench_reset_with_stats();
    borderLine();
    std::cout << "\n";
}