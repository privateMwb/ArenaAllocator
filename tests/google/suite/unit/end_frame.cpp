// Arena endFrame() test suite.
//
// Coverage:
// - Rewinds the allocation cursor to the matching beginFrame() checkpoint
// - Decrements the frame depth
// - Storage freed by the rollback is available for reuse

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies endFrame() restores the cursor to where beginFrame() opened it.
TEST(EndFrame, RewindsCursorToCheckpoint) {
    Arena<> arena(64);
    (void)arena.allocate(8);
    const std::size_t checkpoint = arena.used();

    arena.beginFrame();
    (void)arena.allocate(16);
    arena.endFrame();

    EXPECT_EQ(arena.used(), checkpoint);
}

// Verifies endFrame() decrements frameDepth() by one.
TEST(EndFrame, DecrementsFrameDepth) {
    Arena<> arena(64);
    arena.beginFrame();
    arena.beginFrame();

    arena.endFrame();
    EXPECT_EQ(arena.frameDepth(), 1u);

    arena.endFrame();
    EXPECT_EQ(arena.frameDepth(), 0u);
}

// Verifies bytes rolled back by endFrame() are handed out again.
TEST(EndFrame, ReclaimedSpaceIsReusable) {
    Arena<> arena(64);
    arena.beginFrame();
    std::byte* first = arena.allocate(32);
    arena.endFrame();

    std::byte* second = arena.allocate(32);
    EXPECT_EQ(first, second);
}
