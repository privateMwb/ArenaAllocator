// Arena allocation benchmark suite.
//
// Measures arena allocation performance
//
// Coverage:
// - Single allocation + frame reset
// Against
//   heap allocation and deallocation
// - Aligned allocation
// Against
//   aligned heap allocation and deallocation
// - Sequential allocations
// Against
//   sequential heap allocation and deallocation

#include "bench_helper.h"

using namespace AllocatorPro;

// Measures a single arena allocation + frame reset.
// Against heap allocation and deallocation.
static void bench_allocate() {
    Arena<> a{1024 * 1024};

    auto arena_allocate = [&] {
        a.beginFrame();
        std::byte* ptr = a.allocate(64);
        doNotOptimize(ptr);
        a.endFrame();
        doNotOptimize();
    };
    BENCH("arena_allocate", LARGE, arena_allocate);

    auto heap_allocate = [&] {
        void* ptr = ::operator new(64);
        doNotOptimize(ptr);
        ::operator delete(ptr);
        doNotOptimize();
    };
    BENCH("heap_allocate", LARGE, heap_allocate);
}

// Measures aligned arena allocation.
// Against aligned heap allocation and deallocation.
static void bench_allocate_aligned() {
    Arena<> a{1024 * 1024};

    auto arena_aligned = [&] {
        a.beginFrame();
        std::byte* ptr = a.allocate(64, 64);
        doNotOptimize(ptr);
        a.endFrame();
        doNotOptimize();
    };
    BENCH("arena_allocate_aligned", LARGE, arena_aligned);

    auto heap_aligned = [&] {
        void* ptr = ::operator new(64, std::align_val_t{64});
        doNotOptimize(ptr);
        ::operator delete(ptr, std::align_val_t{64});
        doNotOptimize();
    };
    BENCH("heap_allocate_aligned", LARGE, heap_aligned);
}

// Measures sequential arena allocations.
// Against sequential heap allocation and deallocation.
static void bench_sequential() {
    Arena<> a{1024 * 1024};

    auto arena_sequential = [&] {
        a.beginFrame();

        std::byte* x = a.allocate(64);
        std::byte* y = a.allocate(64);
        std::byte* z = a.allocate(64);

        doNotOptimize(x);
        doNotOptimize(y);
        doNotOptimize(z);

        a.endFrame();
        doNotOptimize();
    };
    BENCH("arena_sequential", LARGE, arena_sequential);

    auto heap_sequential = [&] {
        void* x = ::operator new(64);
        void* y = ::operator new(64);
        void* z = ::operator new(64);

        doNotOptimize(x);
        doNotOptimize(y);
        doNotOptimize(z);

        ::operator delete(x);
        ::operator delete(y);
        ::operator delete(z);

        doNotOptimize();
    };
    BENCH("heap_sequential", LARGE, heap_sequential);
}

// Executes all allocation benchmark cases.
void run_allocation_benchmarks() {
    setHeader("Allocation Benchmarks");

    bench_allocate();
    std::cout << "\n";

    bench_allocate_aligned();
    std::cout << "\n";

    bench_sequential();
    borderLine();
    std::cout << "\n";
}