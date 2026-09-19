// Arena reset() test suite.
//
// Coverage:
// - Restores the allocation cursor and frame depth to zero
// - Clears statistics when EnableStats is true
// - Leaves capacity() unchanged

#include <support/framework.h>

using namespace ArenaPro;

// Verifies reset() zeroes both the cursor and the open-frame count.
static void resets_cursor_and_depth() {
    Arena<> arena(64);
    (void)arena.allocate(16);
    arena.beginFrame();
    (void)arena.allocate(8);

    arena.reset();

    CHK(arena.used() == 0);
    CHK(arena.frameDepth() == 0);
}

// Verifies reset() clears every field of Stats when EnableStats is true.
static void clears_stats_when_enabled() {
    Arena<true> arena(64);
    (void)arena.allocate(16);

    arena.reset();

    const auto& stats = arena.getStats();
    CHK(stats.totalAllocated_ == 0);
    CHK(stats.currentUsed_ == 0);
    CHK(stats.peakUsed_ == 0);
    CHK(stats.allocations_ == 0);
}

// Verifies reset() does not change the buffer's total capacity.
static void capacity_unchanged() {
    Arena<> arena(64);
    (void)arena.allocate(16);

    arena.reset();

    CHK(arena.capacity() == 64);
}

// Executes all reset() test cases.
static void run_tests() {
    RUN(resets_cursor_and_depth);
    RUN(clears_stats_when_enabled);
    RUN(capacity_unchanged);
}

REGISTER_TEST_SUITE();
