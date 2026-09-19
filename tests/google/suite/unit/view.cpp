// Arena view() test suite.
//
// Coverage:
// - Span size matches used()
// - Span starts at the first allocated byte
// - A freshly constructed arena has a zero-size view

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies the returned span's size equals used().
TEST(View, SpanSizeMatchesUsed) {
    Arena<> arena(64);
    (void)arena.allocate(10);
    EXPECT_EQ(arena.view().size(), arena.used());
}

// Verifies the returned span covers exactly the allocated bytes.
TEST(View, SpanCoversAllocatedBytes) {
    Arena<> arena(64);
    std::byte* p = arena.allocate(10);
    EXPECT_EQ(arena.view().data(), p);
}

// Verifies a fresh arena reports an empty view.
TEST(View, EmptyArenaHasZeroSizeView) {
    Arena<> arena(64);
    EXPECT_EQ(arena.view().size(), 0u);
}
