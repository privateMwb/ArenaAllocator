// Arena move semantics test suite.
//
// Coverage:
// - Move construction transfers buffer/cursor/frame state
// - Move construction leaves the source valid and empty
// - Move assignment transfers state and releases the destination's own buffer
// - Move assignment leaves the source valid and empty
// - Self-move-assignment is a safe no-op

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <utility>

using namespace ArenaPro;

// Verifies the destination inherits the source's capacity, cursor, and frames.
TEST(MoveSemantics, MoveConstructTransfersState) {
    Arena<> source(64);
    (void)source.allocate(10);
    source.beginFrame();

    Arena<> dest(std::move(source));
    EXPECT_EQ(dest.capacity(), 64u);
    EXPECT_EQ(dest.used(), 10u);
    EXPECT_EQ(dest.frameDepth(), 1u);
}

// Verifies the moved-from source is left in a valid, empty state.
TEST(MoveSemantics, MoveConstructLeavesSourceEmpty) {
    Arena<> source(64);
    (void)source.allocate(10);

    Arena<> dest(std::move(source));
    (void)dest;

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_EQ(source.capacity(), 0u);
    EXPECT_EQ(source.used(), 0u);
    EXPECT_EQ(source.frameDepth(), 0u);
}

// Verifies move assignment transfers the source's state into an
// already-constructed destination.
TEST(MoveSemantics, MoveAssignTransfersState) {
    Arena<> source(64);
    (void)source.allocate(20);

    Arena<> dest(32);
    dest = std::move(source);

    EXPECT_EQ(dest.capacity(), 64u);
    EXPECT_EQ(dest.used(), 20u);
}

// Verifies the moved-from source is left in a valid, empty state after
// move assignment.
TEST(MoveSemantics, MoveAssignLeavesSourceEmpty) {
    Arena<> source(64);
    (void)source.allocate(20);

    Arena<> dest(32);
    dest = std::move(source);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_EQ(source.capacity(), 0u);
    EXPECT_EQ(source.used(), 0u);
}

// Verifies self-move-assignment does not corrupt or release the arena.
TEST(MoveSemantics, SelfMoveAssignmentIsSafe) {
    Arena<> arena(64);
    (void)arena.allocate(10);

    Arena<>& ref = arena;
    arena = std::move(ref);

    EXPECT_EQ(arena.capacity(), 64u);
    EXPECT_EQ(arena.used(), 10u);
}
