// Arena stats tracking integration test suite.
//
// Coverage:
// - Stats stay correct across a realistic mixed allocate/frame sequence
// - Rolling back a frame updates currentUsed_ but never totalAllocated_
//   or allocations_ (those are lifetime totals, not undone by rollback)

#include <support/framework.h>

using namespace ArenaPro;

// Verifies totals and current usage stay correct through a mixed
// sequence of plain allocations and a rolled-back frame.
static void stats_stay_correct_across_mixed_sequence() {
    Arena<true> arena(128);

    (void)arena.allocate(10);
    (void)arena.allocate(20);
    arena.beginFrame();
    (void)arena.allocate(30);
    arena.endFrame();
    (void)arena.allocate(5);

    const auto& stats = arena.getStats();
    CHK(stats.totalAllocated_ == 65);
    CHK(stats.allocations_ == 4);
    CHK(stats.currentUsed_ == arena.used());
}

// Verifies a frame rollback lowers currentUsed_ without touching the
// lifetime totalAllocated_ counter.
static void frame_rollback_does_not_reduce_total_allocated() {
    Arena<true> arena(128);

    arena.beginFrame();
    (void)arena.allocate(40);
    const std::size_t totalBeforeRollback = arena.getStats().totalAllocated_;

    arena.endFrame();

    CHK(arena.getStats().totalAllocated_ == totalBeforeRollback);
    CHK(arena.getStats().currentUsed_ == 0);
}

// Executes all stats tracking integration test cases.
static void run_tests() {
    RUN(stats_stay_correct_across_mixed_sequence);
    RUN(frame_rollback_does_not_reduce_total_allocated);
}

REGISTER_TEST_SUITE();
