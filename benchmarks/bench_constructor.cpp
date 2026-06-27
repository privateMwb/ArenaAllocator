// Arena Constructor Benchmark Suite
// Measures the cost of arena construction and destruction
// against equivalent heap allocation and deallocation.
//
// Covers:
// - arena construction vs heap allocation of equivalent size
// - arena destruction vs heap deallocation
// - move construction vs heap allocation
// - move assignment vs heap allocation

#include "bench_helper.h"

using namespace AllocatorPro;

// Construct
// measures arena construction vs equivalent heap allocation and deallocation
static void bench_construct() {
    BENCH("arena_construct", LARGE, {
        Arena arena{1024};
        doNotOptimize(arena);
    });

    BENCH("heap_construct", LARGE, {
        void* p = ::operator new(1024);
        doNotOptimize(p);
        ::operator delete(p);
    });
}

// Move Construct
// measures arena move construction vs heap pointer reassignment
static void bench_move_construct() {
    BENCH("arena_move_construct", LARGE, {
        Arena a{1024};
        Arena b{std::move(a)};
        doNotOptimize(b);
    });

    BENCH("heap_move_construct", LARGE, {
        void* a = ::operator new(1024);
        void* b = a;
        doNotOptimize(b);
        ::operator delete(a);
    });
}

// Move Assign
// measures arena move assignment vs heap reallocation and pointer swap
static void bench_move_assign() {
    BENCH("arena_move_assign", LARGE, {
        Arena a{1024};
        Arena b{512};
        b = std::move(a);
        doNotOptimize(b);
    });

    BENCH("heap_move_assign", LARGE, {
        void* a = ::operator new(1024);
        void* b = ::operator new(512);
        ::operator delete(b);
        b = a;
        doNotOptimize(b);
        ::operator delete(a);
    });
}

// Benchmark Runner
// Executes all constructor benchmark cases.
void run_constructor_benchmarks() {
    setHeader("Constructor Benchmarks");

    bench_construct();
    std::cout << "\n";

    bench_move_construct();
    std::cout << "\n";

    bench_move_assign();
    borderLine();
    std::cout << "\n";
}
