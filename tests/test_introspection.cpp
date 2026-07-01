// Arena Introspection Test Suite
// Validates ownership queries, memory views, usage reporting,
// and statistics accessors.
//
// Covers:
// - pointer ownership
// - null and external pointers
// - memory view
// - usage reporting
// - statistics access

#include "test_helper.h"

#include <cstddef>

// Verifies owned pointers are recognized by the arena.
static void owns_valid_pointer() {
    AllocatorPro::Arena<false> arena{1024};
    void* ptr = arena.allocate(64, alignof(std::max_align_t));

    CHK(arena.owns(ptr));
}

// Verifies pointers outside the arena are rejected.
static void owns_out_of_range() {
    AllocatorPro::Arena<false> arena{1024};
    int x = 42;

    CHK(!arena.owns(&x));
}

// Verifies nullptr is never considered owned.
static void owns_nullptr() {
    AllocatorPro::Arena<false> arena{1024};

    CHK(!arena.owns(nullptr));
}

// Verifies the memory view reflects the current allocation size.
static void view_length_matches_used() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(128, alignof(std::max_align_t));

    CHK(arena.view().size() == arena.used());
}

// Verifies the memory view is empty before any allocation.
static void view_empty_before_allocation() {
    AllocatorPro::Arena<false> arena{1024};

    CHK(arena.view().size() == 0);
}

// Verifies the memory view grows as allocations are made.
static void view_grows_with_allocations() {
    AllocatorPro::Arena<false> arena{1024};

    (void)arena.allocate(64, alignof(std::max_align_t));
    const std::size_t first = arena.view().size();

    (void)arena.allocate(64, alignof(std::max_align_t));
    const std::size_t second = arena.view().size();

    CHK(second > first);
}

// Verifies capacity, used, and remaining stay consistent.
static void capacity_used_remaining_consistent() {
    AllocatorPro::Arena<false> arena{1024};

    CHK(arena.capacity() == arena.used() + arena.remaining());

    (void)arena.allocate(256, alignof(std::max_align_t));
    CHK(arena.capacity() == arena.used() + arena.remaining());

    arena.reset();
    CHK(arena.capacity() == arena.used() + arena.remaining());
}

// Verifies statistics remain consistent after multiple allocations.
static void stats_reference_valid() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(64,  alignof(std::max_align_t));
    (void)arena.allocate(128, alignof(std::max_align_t));

    const auto& s = arena.getStats();

    CHK(s.allocations_    == 2);
    CHK(s.totalAllocated_ >= 192);
    CHK(s.currentUsed_    == arena.used());
    CHK(s.peakUsed_       >= s.currentUsed_);
}

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