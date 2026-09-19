// Arena reset() test suite.
//
// Coverage:
// - Restores the allocation cursor and frame depth to zero
// - Clears statistics when EnableStats is true
// - Leaves capacity() unchanged

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies reset() zeroes both the cursor and the open-frame count.
TEST(Reset, ResetsCursorAndDepth) {
    Arena<> arena(64);
    (void)arena.allocate(16);
    arena.beginFrame();
    (void)arena.allocate(8);

    arena.reset();

    EXPECT_EQ(arena.used(), 0u);
    EXPECT_EQ(arena.frameDepth(), 0u);
}

// Verifies reset() clears every field of Stats when EnableStats is true.
TEST(Reset, ClearsStatsWhenEnabled) {
    Arena<true> arena(64);
    (void)arena.allocate(16);

    arena.reset();

    const auto& stats = arena.getStats();
    EXPECT_EQ(stats.totalAllocated_, 0u);
    EXPECT_EQ(stats.currentUsed_, 0u);
    EXPECT_EQ(stats.peakUsed_, 0u);
    EXPECT_EQ(stats.allocations_, 0u);
}

// Verifies reset() does not change the buffer's total capacity.
TEST(Reset, CapacityUnchanged) {
    Arena<> arena(64);
    (void)arena.allocate(16);

    arena.reset();

    EXPECT_EQ(arena.capacity(), 64u);
}
