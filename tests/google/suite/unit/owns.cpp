// Arena owns() test suite.
//
// Coverage:
// - Pointer to a live allocation is reported as owned
// - Pointer outside the buffer is reported as not owned
// - One-past-the-end pointer is reported as not owned (half-open range)
// - nullptr is reported as not owned

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies a pointer returned by allocate() is reported as owned.
TEST(Owns, LiveAllocationIsOwned) {
    Arena<> arena(64);
    std::byte* p = arena.allocate(16);
    EXPECT_TRUE(arena.owns(p));
}

// Verifies a pointer from an unrelated buffer is reported as not owned.
TEST(Owns, ForeignPointerIsNotOwned) {
    Arena<> arena(64);
    std::byte other[16];
    EXPECT_FALSE(arena.owns(other));
}

// Verifies the one-past-the-end address is excluded (range is half-open).
TEST(Owns, EndPointerIsNotOwned) {
    Arena<> arena(64);
    std::byte* p = arena.allocate(64);
    EXPECT_TRUE(arena.owns(p));
    EXPECT_FALSE(arena.owns(p + 64));
}

// Verifies nullptr is reported as not owned.
TEST(Owns, NullPointerIsNotOwned) {
    Arena<> arena(64);
    EXPECT_FALSE(arena.owns(nullptr));
}
