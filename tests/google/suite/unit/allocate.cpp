// Arena allocate() test suite.
//
// Coverage:
// - Returns a non-null pointer when capacity allows
// - Returns nullptr when the request exceeds remaining capacity
// - Successive allocations advance the cursor
// - A zero-size request succeeds (allowed per contract)

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies a request that fits in the buffer succeeds.
TEST(Allocate, ReturnsPointerWithinCapacity) {
    Arena<> arena(64);
    std::byte* p = arena.allocate(16);
    EXPECT_NE(p, nullptr);
}

// Verifies a request larger than the remaining capacity fails.
TEST(Allocate, ReturnsNullptrWhenOutOfSpace) {
    Arena<> arena(8);
    std::byte* p = arena.allocate(16);
    EXPECT_EQ(p, nullptr);
}

// Verifies the bump-pointer cursor moves forward after each allocation.
TEST(Allocate, SuccessiveAllocationsAdvanceCursor) {
    Arena<> arena(64);
    const std::size_t before = arena.used();

    (void)arena.allocate(8);
    const std::size_t afterFirst = arena.used();
    EXPECT_GT(afterFirst, before);

    (void)arena.allocate(8);
    EXPECT_GT(arena.used(), afterFirst);
}

// Verifies a zero-size request is accepted and returns a valid pointer.
TEST(Allocate, ZeroSizeAllocationSucceeds) {
    Arena<> arena(64);
    std::byte* p = arena.allocate(0);
    EXPECT_NE(p, nullptr);
}
