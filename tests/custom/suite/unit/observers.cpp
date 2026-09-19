// Arena observers test suite.
//
// Coverage:
// - used() reflects bytes allocated so far
// - remaining() equals capacity() minus used()
// - capacity() matches the constructor argument
// - frameDepth() reflects the number of open frames
// - getStats() tracks running totals when EnableStats is true

#include <support/framework.h>

using namespace ArenaPro;

// Verifies used() starts at zero and grows with each allocation.
static void used_reflects_allocations() {
    Arena<> arena(64);
    CHK(arena.used() == 0);

    (void)arena.allocate(10);
    CHK(arena.used() == 10);
}

// Verifies remaining() is always capacity() - used().
static void remaining_reflects_capacity_minus_used() {
    Arena<> arena(64);
    (void)arena.allocate(10);
    CHK(arena.remaining() == arena.capacity() - arena.used());
}

// Verifies capacity() reports the size passed to the constructor.
static void capacity_matches_constructor_arg() {
    Arena<> arena(128);
    CHK(arena.capacity() == 128);
}

// Verifies frameDepth() tracks the number of currently open frames.
static void frame_depth_reflects_open_frames() {
    Arena<> arena(64);
    arena.beginFrame();
    arena.beginFrame();
    CHK(arena.frameDepth() == 2);

    arena.endFrame();
    arena.endFrame();
    CHK(arena.frameDepth() == 0);
}

// Verifies getStats() accumulates totals across several allocations.
static void get_stats_tracks_totals() {
    Arena<true> arena(64);
    (void)arena.allocate(10);
    (void)arena.allocate(20);

    const auto& stats = arena.getStats();
    CHK(stats.totalAllocated_ == 30);
    CHK(stats.allocations_ == 2);
    CHK(stats.currentUsed_ == arena.used());
    CHK(stats.peakUsed_ == arena.used());
}

// Executes all observer test cases.
static void run_tests() {
    RUN(used_reflects_allocations);
    RUN(remaining_reflects_capacity_minus_used);
    RUN(capacity_matches_constructor_arg);
    RUN(frame_depth_reflects_open_frames);
    RUN(get_stats_tracks_totals);
}

REGISTER_TEST_SUITE();
