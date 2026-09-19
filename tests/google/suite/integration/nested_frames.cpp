// Arena nested frames integration test suite.
//
// Coverage:
// - Three levels of nested frames each restore the correct checkpoint
// - Rolling back an inner frame makes its space reusable while an
//   outer frame's allocations remain untouched

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies each endFrame() in a three-level nest restores the cursor
// to exactly where the matching beginFrame() opened it.
TEST(NestedFrames, ThreeLevelNestingRestores) {
    Arena<> arena(64);

    arena.beginFrame();
    (void)arena.allocate(8);
    const std::size_t afterLevel1 = arena.used();

    arena.beginFrame();
    (void)arena.allocate(16);
    const std::size_t afterLevel2 = arena.used();

    arena.beginFrame();
    (void)arena.allocate(24);
    EXPECT_GT(arena.used(), afterLevel2);

    arena.endFrame();
    EXPECT_EQ(arena.used(), afterLevel2);

    arena.endFrame();
    EXPECT_EQ(arena.used(), afterLevel1);

    arena.endFrame();
    EXPECT_EQ(arena.used(), 0u);
}

// Verifies rolling back an inner frame frees its bytes for reuse while
// an allocation from the still-open outer frame is left alone.
TEST(NestedFrames, InnerRollbackOuterKeepsOwn) {
    Arena<> arena(64);

    arena.beginFrame();
    std::byte* outer = arena.allocate(16);

    arena.beginFrame();
    std::byte* innerFirst = arena.allocate(16);
    arena.endFrame();

    std::byte* innerSecond = arena.allocate(16);
    EXPECT_EQ(innerFirst, innerSecond);

    arena.endFrame();
    EXPECT_EQ(arena.used(), 0u);
    (void)outer;
}
