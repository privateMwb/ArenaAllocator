// Arena Core Allocation Test Suite
// Validates fundamental allocation behavior, alignment guarantees,
// overflow handling, and pointer validity after allocation.
//
// Covers:
// - basic allocation returns non-null pointer
// - allocation advances offset correctly
// - alignment is respected for various alignments
// - over-capacity allocation returns nullptr
// - multiple allocations are contiguous and non-overlapping
// - typed allocate<T> returns correctly aligned pointer
// - stats update correctly after allocation

#include "test_helper.h"

#include <cstddef>

// Basic Allocation
// verifies allocation returns a non-null pointer for valid inputs
static void basic_allocation() {
    AllocatorPro::Arena arena{1024};
    void* ptr = arena.allocate(64, alignof(std::max_align_t));

    CHK(ptr != nullptr);
}

// Offset Advance
// verifies used() increases by at least the requested size after allocation
static void offset_advance() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    CHK(arena.used() >= 64);
    CHK(arena.remaining() <= 960);
}

// Alignment Respected
// verifies returned pointer satisfies the requested alignment
static void alignment_respected() {
    AllocatorPro::Arena arena{1024};

    void* p1 = arena.allocate(1,  1);
    void* p2 = arena.allocate(4,  4);
    void* p4 = arena.allocate(16, 16);

    CHK(reinterpret_cast<std::uintptr_t>(p1) % 1  == 0);
    CHK(reinterpret_cast<std::uintptr_t>(p2) % 4  == 0);
    CHK(reinterpret_cast<std::uintptr_t>(p4) % 16 == 0);
}

// Over Capacity
// verifies allocation beyond capacity returns nullptr
static void over_capacity() {
    AllocatorPro::Arena arena{64};
    void* ptr = arena.allocate(128, alignof(std::max_align_t));

    CHK(ptr == nullptr);
    CHK(arena.used() == 0);
}

// Exact Capacity
// verifies allocation of exactly the full capacity succeeds
static void exact_capacity() {
    AllocatorPro::Arena arena{64};
    void* ptr = arena.allocate(64, 1);

    CHK(ptr != nullptr);
    CHK(arena.used()      == 64);
    CHK(arena.remaining() == 0);
}

// Non Overlapping Allocations
// verifies successive allocations do not overlap in memory
static void non_overlapping_allocations() {
    AllocatorPro::Arena arena{1024};

    void* p1 = arena.allocate(64, alignof(std::max_align_t));
    void* p2 = arena.allocate(64, alignof(std::max_align_t));

    const auto a1 = reinterpret_cast<std::uintptr_t>(p1);
    const auto a2 = reinterpret_cast<std::uintptr_t>(p2);

    CHK(a2 >= a1 + 64);
}

// Typed Allocate
// verifies allocate<T> returns a pointer with correct type alignment
static void typed_allocate() {
    AllocatorPro::Arena arena{1024};

    int*    pi = arena.allocate<int>();
    double* pd = arena.allocate<double>();

    CHK(pi != nullptr);
    CHK(pd != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(pi) % alignof(int)    == 0);
    CHK(reinterpret_cast<std::uintptr_t>(pd) % alignof(double) == 0);
}

// Stats After Allocation
// verifies allocations_ and totalAllocated_ update correctly
static void stats_after_allocation() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(32, alignof(std::max_align_t));
    (void)arena.allocate(64, alignof(std::max_align_t));

    const auto& s = arena.getStats();

    CHK(s.allocations_    == 2);
    CHK(s.totalAllocated_ >= 96);
    CHK(s.currentUsed_    == arena.used());
}

// Peak Used Tracking
// verifies peakUsed_ reflects the highest watermark reached
static void peak_used_tracking() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(256, alignof(std::max_align_t));

    const std::size_t peak = arena.getStats().peakUsed_;

    arena.reset();
    (void)arena.allocate(64, alignof(std::max_align_t));

    CHK(arena.getStats().peakUsed_ == peak);
}

// Test Runner
// Executes all core allocation test cases.
void run_core_allocation_tests() {
    setTitle("Core Allocation Tests");

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