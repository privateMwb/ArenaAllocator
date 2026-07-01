// Arena State Management Test Suite
// Validates reset behavior, allocator reuse, capacity preservation,
// frame state restoration, and statistics updates.
//
// Covers:
// - offset reset
// - statistics reset
// - capacity preservation
// - allocator reuse
// - frame state reset

#include "test_helper.h"

// Verifies reset clears the allocation offset.
static void reset_clears_offset() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(256, alignof(std::max_align_t));

    arena.reset();

    CHK(arena.used() == 0);
}

// verifies reset zeroes out currentUsed_ and allocations_
static void reset_clears_stats() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(128, alignof(std::max_align_t));

    arena.reset();

    CHK(arena.getStats().currentUsed_ == 0);
    CHK(arena.getStats().allocations_ == 0);
}

// Verifies reset preserves the arena capacity.
static void reset_preserves_capacity() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(512, alignof(std::max_align_t));

    arena.reset();

    CHK(arena.capacity() == 1024);
}

// Verifies allocations restart from the beginning after reset.
static void reset_allows_reallocation() {
    AllocatorPro::Arena<false> arena{1024};

    void* p1 = arena.allocate(512, alignof(std::max_align_t));
    arena.reset();
    void* p2 = arena.allocate(512, alignof(std::max_align_t));

    CHK(p1 == p2);
}

// Verifies reset clears all active frame state.
static void reset_clears_frame_depth() {
    AllocatorPro::Arena<false> arena{1024};

    arena.beginFrame();
    arena.beginFrame();
    arena.reset();

    // After reset, frame operations should work normally.
    arena.beginFrame();
    arena.endFrame();

    CHK(arena.used() == 0);
}

// Executes all state management test cases.
void run_state_tests() {
    setTitle("State Management Tests");

    RUN(reset_clears_offset);
    RUN(reset_clears_stats);
    RUN(reset_preserves_capacity);
    RUN(reset_allows_reallocation);
    RUN(reset_clears_frame_depth);

    std::cout << "\n";
}