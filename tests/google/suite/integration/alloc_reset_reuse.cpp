// Arena allocate/reset/reuse integration test suite.
//
// Coverage:
// - Filling the arena to capacity, then reset(), makes the full
//   capacity available again
// - The first allocation after reset() lands at the same address as
//   the very first allocation ever made

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies reset() restores full capacity after the arena was filled.
TEST(AllocResetReuse, FillCapacityResetReuse) {
    Arena<> arena(32);
    std::byte* p = arena.allocate(32);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(arena.remaining(), 0u);

    arena.reset();
    EXPECT_EQ(arena.remaining(), 32u);

    std::byte* q = arena.allocate(32);
    EXPECT_NE(q, nullptr);
}

// Verifies the cursor restarts from the beginning of the buffer after reset().
TEST(AllocResetReuse, AllocAfterResetStartsBeginning) {
    Arena<> arena(64);
    std::byte* first = arena.allocate(16);

    arena.reset();
    std::byte* afterReset = arena.allocate(16);

    EXPECT_EQ(first, afterReset);
}
