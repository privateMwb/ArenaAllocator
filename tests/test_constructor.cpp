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

// Verifies the arena initializes with the expected capacity and empty state.
static void basic_construction() {
    AllocatorPro::Arena<false> arena{1024};

    CHK(arena.capacity()  == 1024);
    CHK(arena.used()      == 0);
    CHK(arena.remaining() == 1024);
}

// Verifies allocation statistics are zero immediately after construction.
static void initial_stats() {
    AllocatorPro::Arena<true> arena{1024};
    const auto& s = arena.getStats();

    CHK(s.totalAllocated_ == 0);
    CHK(s.currentUsed_    == 0);
    CHK(s.peakUsed_       == 0);
    CHK(s.allocations_    == 0);
}

// Verifies move construction transfers allocator ownership to the destination.
static void move_construction() {
    AllocatorPro::Arena<false> a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    AllocatorPro::Arena<false> b{std::move(a)};

    CHK(b.capacity()  == 512);
    CHK(b.used()      == 64);
    CHK(a.capacity()  == 0);
    CHK(a.used()      == 0);
}

// Verifies move assignment transfers allocator ownership to the destination.
static void move_assignment() {
    AllocatorPro::Arena<false> a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    AllocatorPro::Arena<false> b{128};
    b = std::move(a);

    CHK(b.capacity()  == 512);
    CHK(b.used()      == 64);
    CHK(a.capacity()  == 0);
    CHK(a.used()      == 0);
}

// Verifies self move assignment leaves the allocator in a valid state.
static void self_move_assignment() {
    AllocatorPro::Arena<false> arena{256};
    (void)arena.allocate(32, alignof(std::max_align_t));

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
    arena = std::move(arena);
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    CHK(arena.capacity() == 256);
    CHK(arena.used()     == 32);
}

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