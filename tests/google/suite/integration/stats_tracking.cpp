// Arena stats tracking integration test suite.
//
// Coverage:
// - Stats stay correct across a realistic mixed allocate/frame sequence
// - Rolling back a frame updates currentUsed_ but never totalAllocated_
//   or allocations_ (those are lifetime totals, not undone by rollback)

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies totals and current usage stay correct through a mixed
// sequence of plain allocations and a rolled-back frame.
TEST(StatsTracking, StatsCorrectMixedSequence) {
    Arena<true> arena(128);

    (void)arena.allocate(10);
    (void)arena.allocate(20);
    arena.beginFrame();
    (void)arena.allocate(30);
    arena.endFrame();
    (void)arena.allocate(5);

    const auto& stats = arena.getStats();
    EXPECT_EQ(stats.totalAllocated_, 65u);
    EXPECT_EQ(stats.allocations_, 4u);
    EXPECT_EQ(stats.currentUsed_, arena.used());
}

// Verifies a frame rollback lowers currentUsed_ without touching the
// lifetime totalAllocated_ counter.
TEST(StatsTracking, RollbackDoesNotReduceTotal) {
    Arena<true> arena(128);

    arena.beginFrame();
    (void)arena.allocate(40);
    const std::size_t totalBeforeRollback = arena.getStats().totalAllocated_;

    arena.endFrame();

    EXPECT_EQ(arena.getStats().totalAllocated_, totalBeforeRollback);
    EXPECT_EQ(arena.getStats().currentUsed_, 0u);
}
