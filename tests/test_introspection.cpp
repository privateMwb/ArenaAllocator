// Arena Introspection Test Suite
// Validates ownership queries, memory view, and stat accessors
// across allocated, unallocated, and reset states.
//
// Covers:
// - owns returns true for pointer within arena buffer
// - owns returns false for pointer outside arena buffer
// - owns returns false for nullptr
// - view returns span covering live portion of buffer
// - view length matches used() after allocation
// - capacity, used, remaining are consistent at all times
// - getStats returns correct reference after multiple allocations

#include "test_helper.h"

#include <cstddef>

// Owns Valid Pointer
// verifies owns returns true for a pointer returned by allocate
static void owns_valid_pointer() {
    AllocatorPro::Arena arena{1024};
    void* ptr = arena.allocate(64, alignof(std::max_align_t));

    CHK(arena.owns(ptr));
}

// Owns Out Of Range
// verifies owns returns false for a pointer outside the arena buffer
static void owns_out_of_range() {
    AllocatorPro::Arena arena{1024};
    int x = 42;

    CHK(!arena.owns(&x));
}

// Owns Nullptr
// verifies owns returns false for nullptr
static void owns_nullptr() {
    AllocatorPro::Arena arena{1024};

    CHK(!arena.owns(nullptr));
}

// View Length Matches Used
// verifies view() span length equals used() after allocation
static void view_length_matches_used() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(128, alignof(std::max_align_t));

    CHK(arena.view().size() == arena.used());
}

// View Empty Before Allocation
// verifies view() returns an empty span before any allocation
static void view_empty_before_allocation() {
    AllocatorPro::Arena arena{1024};

    CHK(arena.view().size() == 0);
}

// View Grows With Allocations
// verifies view() span grows as more allocations are made
static void view_grows_with_allocations() {
    AllocatorPro::Arena arena{1024};

    (void)arena.allocate(64, alignof(std::max_align_t));
    const std::size_t first = arena.view().size();

    (void)arena.allocate(64, alignof(std::max_align_t));
    const std::size_t second = arena.view().size();

    CHK(second > first);
}

// Capacity Used Remaining Consistent
// verifies capacity() == used() + remaining() at all times
static void capacity_used_remaining_consistent() {
    AllocatorPro::Arena arena{1024};

    CHK(arena.capacity() == arena.used() + arena.remaining());

    (void)arena.allocate(256, alignof(std::max_align_t));
    CHK(arena.capacity() == arena.used() + arena.remaining());

    arena.reset();
    CHK(arena.capacity() == arena.used() + arena.remaining());
}

// Stats Reference Valid
// verifies getStats() returns consistent values after multiple allocations
static void stats_reference_valid() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(64,  alignof(std::max_align_t));
    (void)arena.allocate(128, alignof(std::max_align_t));

    const auto& s = arena.getStats();

    CHK(s.allocations_    == 2);
    CHK(s.totalAllocated_ >= 192);
    CHK(s.currentUsed_    == arena.used());
    CHK(s.peakUsed_       >= s.currentUsed_);
}

// Test Runner
// Executes all introspection test cases.
void run_introspection_tests() {
    setTitle("Introspection Tests");

    RUN(owns_valid_pointer);
    RUN(owns_out_of_range);
    RUN(owns_nullptr);
    RUN(view_length_matches_used);
    RUN(view_empty_before_allocation);
    RUN(view_grows_with_allocations);
    RUN(capacity_used_remaining_consistent);
    RUN(stats_reference_valid);

    std::cout << "\n";
}