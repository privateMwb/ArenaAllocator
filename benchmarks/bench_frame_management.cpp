// Arena Frame Management Benchmark Suite
// Measures the cost of frame push/pop, scoped frames, and nested frames
// against equivalent heap allocation and deallocation patterns.
//
// Covers:
// - beginFrame/endFrame vs heap alloc/dealloc
// - ArenaScope vs heap alloc/dealloc
// - nested frames vs nested heap alloc/dealloc
// - frame with allocations vs heap alloc/dealloc per frame

#include "bench_helper.h"

using namespace AllocatorPro;

// Begin End Frame
// measures beginFrame/endFrame cycle vs heap allocation and deallocation
static void bench_begin_end_frame() {
    BENCH("arena_begin_end_frame", LARGE, {
        Arena arena{1024};
        arena.beginFrame();
        arena.endFrame();
    });

    BENCH("heap_begin_end_frame", LARGE, {
        void* p = ::operator new(1024);
        doNotOptimize(p);
        ::operator delete(p);
    });
}

// Arena Scope
// measures ArenaScope RAII vs heap allocation and deallocation
static void bench_arena_scope() {
    BENCH("arena_scope", LARGE, {
        Arena arena{1024};
        {
            ArenaScope scope{arena};
        }
        doNotOptimize(arena);
    });

    BENCH("heap_scope", LARGE, {
        void* p = ::operator new(1024);
        doNotOptimize(p);
        ::operator delete(p);
    });
}

// Nested Frames
// measures nested beginFrame/endFrame pairs vs nested heap alloc/dealloc
static void bench_nested_frames() {
    BENCH("arena_nested_frames", MEDIUM, {
        Arena arena{1024};
        arena.beginFrame();
        arena.beginFrame();
        arena.beginFrame();
        arena.endFrame();
        arena.endFrame();
        arena.endFrame();
        doNotOptimize(arena);
    });

    BENCH("heap_nested_frames", MEDIUM, {
        void* p1 = ::operator new(256);
        void* p2 = ::operator new(256);
        void* p3 = ::operator new(256);
        doNotOptimize(p1);
        doNotOptimize(p2);
        doNotOptimize(p3);
        ::operator delete(p3);
        ::operator delete(p2);
        ::operator delete(p1);
    });
}

// Frame With Allocations
// measures allocations within a frame vs heap alloc/dealloc per frame
static void bench_frame_with_allocations() {
    BENCH("arena_frame_with_allocs", SMALL, {
        Arena arena{sizeof(Item) * 10 * 2};
        arena.beginFrame();
        for (int j = 0; j < 10; ++j) {
            Item* p = arena.create<Item>(j, float(j), double(j));
            doNotOptimize(p);
        }
        arena.endFrame();
    });

    BENCH("heap_frame_with_allocs", SMALL, {
        Item* ptrs[10];
        for (int j = 0; j < 10; ++j) {
            ptrs[j] = new Item(j, float(j), double(j));
            doNotOptimize(ptrs[j]);
        }
        for (int j = 0; j < 10; ++j)
            delete ptrs[j];
    });
}

// Scope With Allocations
// measures ArenaScope with allocations vs heap alloc/dealloc per scope
static void bench_scope_with_allocations() {
    BENCH("arena_scope_with_allocs", SMALL, {
        Arena arena{sizeof(Item) * 10 * 2};
        {
            ArenaScope scope{arena};
            for (int j = 0; j < 10; ++j) {
                Item* p = arena.create<Item>(j, float(j), double(j));
                doNotOptimize(p);
            }
        }
    });

    BENCH("heap_scope_with_allocs", SMALL, {
        Item* ptrs[10];
        for (int j = 0; j < 10; ++j) {
            ptrs[j] = new Item(j, float(j), double(j));
            doNotOptimize(ptrs[j]);
        }
        for (int j = 0; j < 10; ++j)
            delete ptrs[j];
    });
}

// Benchmark Runner
// Executes all frame management benchmark cases.
void run_frame_management_benchmarks() {
    setHeader("Frame Management Benchmarks");

    bench_begin_end_frame();
    std::cout << "\n";

    bench_arena_scope();
    std::cout << "\n";

    bench_nested_frames();
    std::cout << "\n";

    bench_frame_with_allocations();
    std::cout << "\n";

    bench_scope_with_allocations();
    borderLine();
    std::cout << "\n";
}
