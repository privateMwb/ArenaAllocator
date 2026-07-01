// ArenaScope benchmarks.
//
// Measures the overhead of the ArenaScope RAII wrapper against
// raw beginFrame()/endFrame() usage and equivalent heap patterns.
//
// Coverage:
// - ArenaScope vs raw beginFrame/endFrame
// - ArenaScope vs heap allocation with manual cleanup
// - Nested ArenaScope vs nested raw frames

#include "bench_helper.h"

using namespace AllocatorPro;

// Measures ArenaScope RAII overhead against raw beginFrame/endFrame.
static void bench_scope_vs_raw_frame() {
    Arena<> a{1024 * 1024};

    auto scope_wrapped = [&] {
        ArenaScope<false> scope{a};
        std::byte* ptr = a.allocate(64);
        doNotOptimize(ptr);
    };
    BENCH("arena_scope", LARGE, scope_wrapped);

    auto raw_frame = [&] {
        a.beginFrame();
        std::byte* ptr = a.allocate(64);
        doNotOptimize(ptr);
        a.endFrame();
    };
    BENCH("arena_raw_frame", LARGE, raw_frame);
}

// Measures ArenaScope allocation against heap allocation with manual cleanup.
static void bench_scope_vs_heap() {
    Arena<> a{1024 * 1024};

    auto scope_alloc = [&] {
        ArenaScope<false> scope{a};
        std::byte* ptr = a.allocate(64);
        doNotOptimize(ptr);
    };
    BENCH("arena_scope_alloc", LARGE, scope_alloc);

    auto heap_alloc = [&] {
        void* ptr = ::operator new(64);
        doNotOptimize(ptr);
        ::operator delete(ptr);
    };
    BENCH("heap_alloc", LARGE, heap_alloc);
}

// Measures nested ArenaScope overhead against nested raw frames.
static void bench_nested_scope() {
    Arena<> a{1024 * 1024};

    auto nested_scope = [&] {
        ArenaScope<false> outer{a};
        std::byte* x = a.allocate(64);
        doNotOptimize(x);

        ArenaScope<false> inner{a};
        std::byte* y = a.allocate(64);
        doNotOptimize(y);
    };
    BENCH("arena_nested_scope", LARGE, nested_scope);

    auto nested_raw = [&] {
        a.beginFrame();
        std::byte* x = a.allocate(64);
        doNotOptimize(x);

        a.beginFrame();
        std::byte* y = a.allocate(64);
        doNotOptimize(y);

        a.endFrame();
        a.endFrame();
    };
    BENCH("arena_nested_raw_frame", LARGE, nested_raw);
}

// Runs all ArenaScope benchmark cases.
void run_scope_benchmarks() {
    setHeader("ArenaScope Benchmarks");

    bench_scope_vs_raw_frame();
    std::cout << "\n";

    bench_scope_vs_heap();
    std::cout << "\n";

    bench_nested_scope();
    borderLine();
    std::cout << "\n";
}