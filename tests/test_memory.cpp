// Arena Memory Allocation Test Suite
// Validates allocation behavior, alignment guarantees, capacity
// limits, pointer correctness, and allocation statistics.
//
// Covers:
// - basic allocation
// - offset advancement
// - alignment guarantees
// - over-capacity allocation
// - exact-capacity allocation
// - non-overlapping allocations
// - typed allocation
// - allocation statistics
// - peak usage tracking

#include "test_helper.h"

#include <cstddef>

// Verifies a valid allocation returns a non-null pointer.
static void basic_allocation() {
    AllocatorPro::Arena<false> arena{1024};
    void* ptr = arena.allocate(64, alignof(std::max_align_t));

    CHK(ptr != nullptr);
}

// Verifies allocation advances the arena offset.
static void offset_advance() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    CHK(arena.used() >= 64);
    CHK(arena.remaining() <= 960);
}

// Verifies returned pointers satisfy the requested alignment.
static void alignment_respected() {
    AllocatorPro::Arena<false> arena{1024};

    void* p1 = arena.allocate(1, 1);
    void* p2 = arena.allocate(4, 4);
    void* p4 = arena.allocate(16, 16);

    CHK(reinterpret_cast<std::uintptr_t>(p1) % 1  == 0);
    CHK(reinterpret_cast<std::uintptr_t>(p2) % 4  == 0);
    CHK(reinterpret_cast<std::uintptr_t>(p4) % 16 == 0);
}

// Verifies allocations exceeding capacity fail.
static void over_capacity() {
    AllocatorPro::Arena<false> arena{64};
    void* ptr = arena.allocate(128, alignof(std::max_align_t));

    CHK(ptr == nullptr);
    CHK(arena.used() == 0);
}

// Verifies an allocation equal to the arena capacity succeeds.
static void exact_capacity() {
    AllocatorPro::Arena<false> arena{64};
    void* ptr = arena.allocate(64, 1);

    CHK(ptr != nullptr);
    CHK(arena.used()      == 64);
    CHK(arena.remaining() == 0);
}

// Verifies consecutive allocations do not overlap.
static void non_overlapping_allocations() {
    AllocatorPro::Arena<false> arena{1024};

    void* p1 = arena.allocate(64, alignof(std::max_align_t));
    void* p2 = arena.allocate(64, alignof(std::max_align_t));

    const auto a1 = reinterpret_cast<std::uintptr_t>(p1);
    const auto a2 = reinterpret_cast<std::uintptr_t>(p2);

    CHK(a2 >= a1 + 64);
}

// Verifies typed allocations return correctly aligned storage.
static void typed_allocate() {
    AllocatorPro::Arena<false> arena{1024};

    int*    pi = arena.allocate<int>();
    double* pd = arena.allocate<double>();

    CHK(pi != nullptr);
    CHK(pd != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(pi) % alignof(int)    == 0);
    CHK(reinterpret_cast<std::uintptr_t>(pd) % alignof(double) == 0);
}

// Verifies allocation statistics update correctly.
static void stats_after_allocation() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(32, alignof(std::max_align_t));
    (void)arena.allocate(64, alignof(std::max_align_t));

    const auto& s = arena.getStats();

    CHK(s.allocations_    == 2);
    CHK(s.totalAllocated_ >= 96);
    CHK(s.currentUsed_    == arena.used());
}

// Verifies the peak usage watermark is preserved after reset.
static void peak_used_tracking() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(256, alignof(std::max_align_t));

    arena.reset();
    (void)arena.allocate(64, alignof(std::max_align_t));

    CHK(arena.getStats().peakUsed_ == 64);
}

// Executes all memory allocation test cases.
void run_memory_tests() {
    setTitle("Memory Allocation Tests");

    RUN(basic_allocation);
    RUN(offset_advance);
    RUN(alignment_respected);
    RUN(over_capacity);
    RUN(exact_capacity);
    RUN(non_overlapping_allocations);
    RUN(typed_allocate);
    RUN(stats_after_allocation);
    RUN(peak_used_tracking);

    std::cout << "\n";
}