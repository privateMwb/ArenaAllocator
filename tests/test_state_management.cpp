// Arena State Management Test Suite
// Validates reset behavior including offset clearing, stat resetting,
// capacity preservation, and reallocation after reset.
//
// Covers:
// - reset clears offset to zero
// - reset clears frame depth
// - reset clears current used stats
// - reset allows reallocation from the start
// - reset preserves capacity

#include "test_helper.h"

// Reset Clears Offset
// verifies reset brings used() back to zero
static void reset_clears_offset() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(256, alignof(std::max_align_t));

    arena.reset();

    CHK(arena.used() == 0);
}

// Reset Clears Stats
// verifies reset zeroes out currentUsed_ and allocations_
static void reset_clears_stats() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(128, alignof(std::max_align_t));

    arena.reset();

    CHK(arena.getStats().currentUsed_ == 0);
    CHK(arena.getStats().allocations_ == 0);
}

// Reset Preserves Capacity
// verifies reset does not change the total capacity of the arena
static void reset_preserves_capacity() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(512, alignof(std::max_align_t));

    arena.reset();

    CHK(arena.capacity() == 1024);
}

// Reset Allows Reallocation
// verifies allocation after reset starts from the beginning of the buffer
static void reset_allows_reallocation() {
    AllocatorPro::Arena arena{1024};

    void* p1 = arena.allocate(512, alignof(std::max_align_t));
    arena.reset();
    void* p2 = arena.allocate(512, alignof(std::max_align_t));

    CHK(p1 == p2);
}

// Reset Clears Frame Depth
// verifies reset brings frame depth to zero even if frames were open
static void reset_clears_frame_depth() {
    AllocatorPro::Arena arena{1024};

    arena.beginFrame();
    arena.beginFrame();
    arena.reset();

    // after reset, beginFrame should succeed without overflow
    arena.beginFrame();
    arena.endFrame();

    CHK(arena.used() == 0);
}

// Test Runner
// Executes all state management test cases.
void run_state_management_tests() {
    setTitle("State Management Tests");

    RUN(reset_clears_offset);
    RUN(reset_clears_stats);
    RUN(reset_preserves_capacity);
    RUN(reset_allows_reallocation);
    RUN(reset_clears_frame_depth);

    std::cout << "\n";
}