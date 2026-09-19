// Arena beginFrame() test suite.
//
// Coverage:
// - Opening a frame increments the frame depth
// - Nested frames are tracked correctly

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies beginFrame() increments frameDepth() by one.
TEST(BeginFrame, OpensFrameIncrementsDepth) {
    Arena<> arena(64);
    EXPECT_EQ(arena.frameDepth(), 0u);

    arena.beginFrame();
    EXPECT_EQ(arena.frameDepth(), 1u);

    arena.endFrame();
}

// Verifies several nested beginFrame() calls each increment the depth.
TEST(BeginFrame, NestedFramesTrackDepth) {
    Arena<> arena(64);
    arena.beginFrame();
    arena.beginFrame();
    arena.beginFrame();
    EXPECT_EQ(arena.frameDepth(), 3u);

    arena.endFrame();
    arena.endFrame();
    arena.endFrame();
}
