// Arena observers test suite.
//
// Coverage:
// - used() reflects bytes allocated so far
// - remaining() equals capacity() minus used()
// - capacity() matches the constructor argument
// - frameDepth() reflects the number of open frames
// - getStats() tracks running totals when EnableStats is true

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies used() starts at zero and grows with each allocation.
TEST(Observers, UsedReflectsAllocations) {
    Arena<> arena(64);
    EXPECT_EQ(arena.used(), 0u);

    (void)arena.allocate(10);
    EXPECT_EQ(arena.used(), 10u);
}

// Verifies remaining() is always capacity() - used().
TEST(Observers, RemainingReflectsCapacityMinusUsed) {
    Arena<> arena(64);
    (void)arena.allocate(10);
    EXPECT_EQ(arena.remaining(), arena.capacity() - arena.used());
}

// Verifies capacity() reports the size passed to the constructor.
TEST(Observers, CapacityMatchesConstructorArg) {
    Arena<> arena(128);
    EXPECT_EQ(arena.capacity(), 128u);
}

// Verifies frameDepth() tracks the number of currently open frames.
TEST(Observers, FrameDepthReflectsOpenFrames) {
    Arena<> arena(64);
    arena.beginFrame();
    arena.beginFrame();
    EXPECT_EQ(arena.frameDepth(), 2u);

    arena.endFrame();
    arena.endFrame();
    EXPECT_EQ(arena.frameDepth(), 0u);
}

// Verifies getStats() accumulates totals across several allocations.
TEST(Observers, GetStatsTracksTotals) {
    Arena<true> arena(64);
    (void)arena.allocate(10);
    (void)arena.allocate(20);

    const auto& stats = arena.getStats();
    EXPECT_EQ(stats.totalAllocated_, 30u);
    EXPECT_EQ(stats.allocations_, 2u);
    EXPECT_EQ(stats.currentUsed_, arena.used());
    EXPECT_EQ(stats.peakUsed_, arena.used());
}
