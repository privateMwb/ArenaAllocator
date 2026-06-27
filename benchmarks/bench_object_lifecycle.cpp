// Arena Object Lifecycle Benchmark Suite
// Measures the cost of object creation and destruction
// against equivalent heap allocation and deallocation.
//
// Covers:
// - create<T> vs new T
// - destroy<T> vs delete T
// - create and destroy cycle vs new and delete cycle
// - multiple creates vs multiple new
// - create with complex constructor vs new with complex constructor

#include "bench_helper.h"

using namespace AllocatorPro;

// Create
// measures arena create<Item> vs heap new Item
static void bench_create() {
    BENCH("arena_create", LARGE, [&] {
        Arena arena{sizeof(Item) * 2};
        Item* p = arena.create<Item>(1, 1.0f, 1.0);
        doNotOptimize(p);
    });

    BENCH("heap_create", LARGE, [&] {
        Item* p = new Item(1, 1.0f, 1.0);
        doNotOptimize(p);
        delete p;
    });
}

// Destroy
// measures arena destroy<Item> vs heap delete Item
static void bench_destroy() {
    BENCH("arena_destroy", LARGE, [&] {
        Arena arena{sizeof(Item) * 2};
        Item* p = arena.create<Item>(1, 1.0f, 1.0);
        arena.destroy(p);
        doNotOptimize(p);
    });

    BENCH("heap_destroy", LARGE, [&] {
        Item* p = new Item(1, 1.0f, 1.0);
        doNotOptimize(p);
        delete p;
    });
}

// Create Destroy Cycle
// measures full arena create/destroy cycle vs heap new/delete cycle
static void bench_create_destroy_cycle() {
    BENCH("arena_create_destroy_cycle", MEDIUM, [&] {
        Arena arena{sizeof(Item) * 2};
        Item* p = arena.create<Item>(1, 1.0f, 1.0);
        doNotOptimize(p);
        arena.destroy(p);
    });

    BENCH("heap_create_destroy_cycle", MEDIUM, [&] {
        Item* p = new Item(1, 1.0f, 1.0);
        doNotOptimize(p);
        delete p;
    });
}

// Multiple Creates
// measures multiple arena creates vs multiple heap news
static void bench_multiple_creates() {
    BENCH("arena_multiple_creates", SMALL, [&] {
        Arena arena{sizeof(Item) * 10 * 2};
        for (int j = 0; j < 10; ++j) {
            Item* p = arena.create<Item>(j, float(j), double(j));
            doNotOptimize(p);
        }
    });

    BENCH("heap_multiple_creates", SMALL, [&] {
        Item* ptrs[10];
        for (int j = 0; j < 10; ++j) {
            ptrs[j] = new Item(j, float(j), double(j));
            doNotOptimize(ptrs[j]);
        }

        for (int j = 0; j < 10; ++j)
            delete ptrs[j];
    });
}

// Create With Probe
// measures arena create/destroy with non-trivial destructor vs heap equivalent
static void bench_create_with_probe() {
    BENCH("arena_create_probe", MEDIUM, [&] {
        Arena arena{1024};
        bool destroyed = false;
        Probe* p = arena.create<Probe>(1, destroyed);
        doNotOptimize(p);
        arena.destroy(p);
    });

    BENCH("heap_create_probe", MEDIUM, [&] {
        bool destroyed = false;
        Probe* p = new Probe(1, destroyed);
        doNotOptimize(p);
        delete p;
    });
}

// Benchmark Runner
// Executes all object lifecycle benchmark cases.
void run_object_lifecycle_benchmarks() {
    setHeader("Object Lifecycle Benchmarks");

    bench_create();
    std::cout << "\n";

    bench_destroy();
    std::cout << "\n";

    bench_create_destroy_cycle();
    std::cout << "\n";

    bench_multiple_creates();
    std::cout << "\n";

    bench_create_with_probe();
    borderLine();
    std::cout << "\n";
}
