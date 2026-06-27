// Arena Constructor Test Suite
// Validates arena construction, initialization, and move semantics
// across various construction and ownership transfer scenarios.
//
// Covers:
// - default construction with specified capacity
// - initial statistics after construction
// - move constructor ownership transfer
// - move assignment ownership transfer
// - self move assignment safety

#include "test_helper.h"

#include <cstddef>

// Basic Construction
// verifies arena initializes with correct capacity and empty state
static void basic_construction() {
    AllocatorPro::Arena arena{1024};

    CHK(arena.capacity()  == 1024);
    CHK(arena.used()      == 0);
    CHK(arena.remaining() == 1024);
}

// Initial Stats
// verifies all stats are zero after construction
static void initial_stats() {
    AllocatorPro::Arena arena{1024};
    const auto& s = arena.getStats();

    CHK(s.totalAllocated_ == 0);
    CHK(s.currentUsed_    == 0);
    CHK(s.peakUsed_       == 0);
    CHK(s.allocations_    == 0);
}

// Move Construction
// verifies move constructor transfers ownership and nulls the source
static void move_construction() {
    AllocatorPro::Arena a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    AllocatorPro::Arena b{std::move(a)};

    CHK(b.capacity()  == 512);
    CHK(b.used()      == 64);
    CHK(a.capacity()  == 0);
    CHK(a.used()      == 0);
}

// Move Assignment
// verifies move assignment transfers ownership and nulls the source
static void move_assignment() {
    AllocatorPro::Arena a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    AllocatorPro::Arena b{128};
    b = std::move(a);

    CHK(b.capacity()  == 512);
    CHK(b.used()      == 64);
    CHK(a.capacity()  == 0);
    CHK(a.used()      == 0);
}

// Self Move Assignment
// verifies self-assignment is handled safely
static void self_move_assignment() {
    AllocatorPro::Arena arena{256};
    (void)arena.allocate(32, alignof(std::max_align_t));

    arena = std::move(arena);

    CHK(arena.capacity() == 256);
    CHK(arena.used()     == 32);
}

// Test Runner
// Executes all constructor test cases.
void run_constructor_tests() {
    setTitle("Constructor Tests");

    RUN(basic_construction);
    RUN(initial_stats);
    RUN(move_construction);
    RUN(move_assignment);
    RUN(self_move_assignment);

    std::cout << "\n";
}