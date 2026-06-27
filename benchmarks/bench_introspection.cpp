// Arena Introspection Benchmark Suite
// Measures the cost of ownership queries, memory view, and stat accessors
// against equivalent heap-based checks.
//
// Covers:
// - owns vs manual bounds check
// - view vs heap pointer arithmetic
// - getStats vs manual stat tracking
// - capacity/used/remaining vs heap size tracking

#include "bench_helper.h"

using namespace AllocatorPro;

// Owns
// measures owns() vs equivalent manual bounds check on heap pointer
static void bench_owns() {
    Arena arena{1024};
    void* ptr = arena.allocate(64, alignof(std::max_align_t));

    auto arenaBench = [&] {
        bool result = arena.owns(ptr);
        doNotOptimize(result);
    };

    BENCH("arena_owns", LARGE, arenaBench);

    auto heapBench = [&] {
        void* base = ::operator new(1024);
        bool result = ptr >= base &&
                      ptr < static_cast<std::byte*>(base) + 1024;
        doNotOptimize(result);
        ::operator delete(base);
    };

    BENCH("heap_owns", LARGE, heapBench);
}

// View
// measures view() vs equivalent heap span construction
static void bench_view() {
    Arena arena{1024};
    (void)arena.allocate(256, alignof(std::max_align_t));

    auto arenaBench = [&] {
        auto v = arena.view();
        doNotOptimize(v);
    };

    BENCH("arena_view", LARGE, arenaBench);

    void* base = ::operator new(1024);

    auto heapBench = [&] {
        const std::byte* p = static_cast<const std::byte*>(base);
        std::size_t len = 256;
        doNotOptimize(p);
        doNotOptimize(len);
    };

    BENCH("heap_view", LARGE, heapBench);

    ::operator delete(base);
}

// Get Stats
// measures getStats() vs manual stat struct access
static void bench_get_stats() {
    struct HeapStats {
        std::size_t totalAllocated_;
        std::size_t currentUsed_;
        std::size_t peakUsed_;
        std::size_t allocations_;
    };

    Arena arena{1024};
    (void)arena.allocate(128, alignof(std::max_align_t));

    auto arenaBench = [&] {
        const auto& s = arena.getStats();
        doNotOptimize(s);
    };

    BENCH("arena_get_stats", LARGE, arenaBench);

    auto heapBench = [&] {
        HeapStats s;
        s.totalAllocated_ = 128;
        s.currentUsed_    = 128;
        s.peakUsed_       = 128;
        s.allocations_    = 1;
        doNotOptimize(s);
    };

    BENCH("heap_get_stats", LARGE, heapBench);
}

// Capacity Used Remaining
// measures capacity/used/remaining vs equivalent heap size tracking
static void bench_capacity_used_remaining() {
    Arena arena{1024};
    (void)arena.allocate(256, alignof(std::max_align_t));

    auto arenaBench = [&] {
        std::size_t cap  = arena.capacity();
        std::size_t used = arena.used();
        std::size_t rem  = arena.remaining();
        doNotOptimize(cap);
        doNotOptimize(used);
        doNotOptimize(rem);
    };

    BENCH("arena_capacity_used_remaining", LARGE, arenaBench);

    auto heapBench = [&] {
        std::size_t cap  = 1024;
        std::size_t used = 256;
        std::size_t rem  = cap - used;
        doNotOptimize(cap);
        doNotOptimize(used);
        doNotOptimize(rem);
    };

    BENCH("heap_capacity_used_remaining", LARGE, heapBench);
}

// Benchmark Runner
// Executes all introspection benchmark cases.
void run_introspection_benchmarks() {
    setHeader("Introspection Benchmarks");

    bench_owns();
    std::cout << "\n";

    bench_view();
    std::cout << "\n";

    bench_get_stats();
    std::cout << "\n";

    bench_capacity_used_remaining();
    borderLine();
    std::cout << "\n";
}
