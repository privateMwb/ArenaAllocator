// Arena Core Allocation Benchmark Suite
// Measures the cost of raw allocation, typed allocation, and object creation
// against equivalent heap allocation and deallocation.
//
// Covers:
// - raw allocate vs heap alloc
// - typed allocate<T> vs new T
// - create<T> vs new T with constructor arguments
// - multiple sequential allocations vs multiple heap allocations
// - allocation until full vs heap equivalent

#include "bench_helper.h"

using namespace AllocatorPro;

// Raw Allocate
// measures raw allocate vs heap allocation of equivalent size
static void bench_raw_allocate() {
    BENCH("arena_raw_allocate", LARGE, {
        Arena arena{1024};
        (void)arena.allocate(64, alignof(std::max_align_t));
    });

    BENCH("heap_raw_allocate", LARGE, {
        void* p = ::operator new(64);
        doNotOptimize(p);
        ::operator delete(p);
    });
}

// Typed Allocate
// measures typed allocate<Item> vs new Item without construction
static void bench_typed_allocate() {
    BENCH("arena_typed_allocate", MEDIUM, {
        Arena arena{sizeof(Item) * 2};
        Item* p = arena.allocate<Item>();
        doNotOptimize(p);
    });

    BENCH("heap_typed_allocate", MEDIUM, {
        Item* p = static_cast<Item*>(::operator new(sizeof(Item)));
        doNotOptimize(p);
        ::operator delete(p);
    });
}

// Create
// measures create<Item> with constructor args vs new Item with args
static void bench_create() {
    BENCH("arena_create", MEDIUM, {
        Arena arena{sizeof(Item) * 2};
        Item* p = arena.create<Item>(1, 2.0f, 3.0);
        doNotOptimize(p);
    });

    BENCH("heap_create", MEDIUM, {
        Item* p = new Item(1, 2.0f, 3.0);
        doNotOptimize(p);
        delete p;
    });
}

// Sequential Allocations
// measures multiple sequential arena allocations vs multiple heap allocations
static void bench_sequential_allocations() {
    BENCH("arena_sequential", SMALL, {
        Arena arena{sizeof(Item) * 10 * 2};
        for (int j = 0; j < 10; ++j) {
            Item* p = arena.create<Item>(j, float(j), double(j));
            doNotOptimize(p);
        }
    });

    BENCH("heap_sequential", SMALL, {
        Item* ptrs[10];
        for (int j = 0; j < 10; ++j) {
            ptrs[j] = new Item(j, float(j), double(j));
            doNotOptimize(ptrs[j]);
        }
        for (int j = 0; j < 10; ++j)
            delete ptrs[j];
    });
}

// Allocate Until Full
// measures filling the arena to capacity vs equivalent heap allocations
static void bench_allocate_until_full() {
    constexpr std::size_t kCount    = 16;
    constexpr std::size_t kArenaSize = sizeof(Item) * kCount * 2;

    BENCH("arena_until_full", SMALL, {
        Arena arena{kArenaSize};
        for (std::size_t j = 0; j < kCount; ++j) {
            Item* p = arena.create<Item>(int(j), float(j), double(j));
            doNotOptimize(p);
        }
    });

    BENCH("heap_until_full", SMALL, {
        Item* ptrs[kCount];
        for (std::size_t j = 0; j < kCount; ++j) {
            ptrs[j] = new Item(int(j), float(j), double(j));
            doNotOptimize(ptrs[j]);
        }
        for (std::size_t j = 0; j < kCount; ++j)
            delete ptrs[j];
    });
}

// Benchmark Runner
// Executes all core allocation benchmark cases.
void run_core_allocation_benchmarks() {
    setHeader("Core Allocation Benchmarks");

    bench_raw_allocate();
    std::cout << "\n";

    bench_typed_allocate();
    std::cout << "\n";

    bench_create();
    std::cout << "\n";

    bench_sequential_allocations();
    std::cout << "\n";

    bench_allocate_until_full();
    borderLine();
    std::cout << "\n";
}
