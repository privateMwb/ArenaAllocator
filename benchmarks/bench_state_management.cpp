// Arena State Management Benchmark Suite
// Measures the cost of reset against equivalent heap deallocation patterns.
//
// Covers:
// - reset vs heap deallocation of single allocation
// - reset vs heap deallocation of multiple allocations
// - reset and reallocate vs heap dealloc and realloc
// - repeated reset cycles vs repeated heap alloc/dealloc cycles

#include "bench_helper.h"

using namespace AllocatorPro;

// Reset Single
// measures reset after single allocation vs heap deallocation
static void bench_reset_single() {
    BENCH("arena_reset_single", LARGE, {
        Arena arena{1024};
        (void)arena.allocate(64, alignof(std::max_align_t));
        arena.reset();
        doNotOptimize(arena);
    });

    BENCH("heap_reset_single", LARGE, {
        void* p = ::operator new(64);
        doNotOptimize(p);
        ::operator delete(p);
    });
}

// Reset Multiple
// measures reset after multiple allocations vs heap deallocation of each
static void bench_reset_multiple() {
    BENCH("arena_reset_multiple", MEDIUM, {
        Arena arena{sizeof(Item) * 10 * 2};
        for (int j = 0; j < 10; ++j) {
            Item* p = arena.create<Item>(j, float(j), double(j));
            doNotOptimize(p);
        }
        arena.reset();
    });

    BENCH("heap_reset_multiple", MEDIUM, {
        Item* ptrs[10];
        for (int j = 0; j < 10; ++j) {
            ptrs[j] = new Item(j, float(j), double(j));
            doNotOptimize(ptrs[j]);
        }
        for (int j = 0; j < 10; ++j)
            delete ptrs[j];
    });
}

// Reset And Reallocate
// measures reset followed by reallocation vs heap dealloc and realloc
static void bench_reset_and_reallocate() {
    BENCH("arena_reset_reallocate", MEDIUM, {
        Arena arena{sizeof(Item) * 2};
        Item* p1 = arena.create<Item>(1, 1.0f, 1.0);
        doNotOptimize(p1);
        arena.reset();
        Item* p2 = arena.create<Item>(2, 2.0f, 2.0);
        doNotOptimize(p2);
    });

    BENCH("heap_reset_reallocate", MEDIUM, {
        Item* p1 = new Item(1, 1.0f, 1.0);
        doNotOptimize(p1);
        delete p1;
        Item* p2 = new Item(2, 2.0f, 2.0);
        doNotOptimize(p2);
        delete p2;
    });
}

// Repeated Reset Cycles
// measures repeated reset cycles vs repeated heap alloc/dealloc cycles
static void bench_repeated_reset_cycles() {
    BENCH("arena_repeated_cycles", SMALL, {
        Arena arena{sizeof(Item) * 5 * 2};
        for (int j = 0; j < 5; ++j) {
            Item* p = arena.create<Item>(j, float(j), double(j));
            doNotOptimize(p);
            arena.reset();
        }
    });

    BENCH("heap_repeated_cycles", SMALL, {
        for (int j = 0; j < 5; ++j) {
            Item* p = new Item(j, float(j), double(j));
            doNotOptimize(p);
            delete p;
        }
    });
}

// Benchmark Runner
// Executes all state management benchmark cases.
void run_state_management_benchmarks() {
    setHeader("State Management Benchmarks");

    bench_reset_single();
    std::cout << "\n";

    bench_reset_multiple();
    std::cout << "\n";

    bench_reset_and_reallocate();
    std::cout << "\n";

    bench_repeated_reset_cycles();
    borderLine();
    std::cout << "\n";
}
