// Arena / ArenaScope integration test suite.
//
// Coverage:
// - ArenaScope opens a frame on construction, closes it on destruction
// - The frame is rolled back even when the scoped code throws
// - Storage freed by the rollback is handed out again

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <stdexcept>

using namespace ArenaPro;

// Verifies the scope's RAII frame tracks arena.frameDepth() correctly.
TEST(ScopeRollback, OpensAndClosesFrameViaRaii) {
    Arena<> arena(64);
    EXPECT_EQ(arena.frameDepth(), 0u);
    {
        ArenaScope<false> scope(arena);
        EXPECT_EQ(arena.frameDepth(), 1u);
    }
    EXPECT_EQ(arena.frameDepth(), 0u);
}

// Verifies an exception unwinding through a scope still rolls back its frame.
TEST(ScopeRollback, RollsBackOnException) {
    Arena<> arena(64);
    const std::size_t before = arena.used();

    try {
        ArenaScope<false> scope(arena);
        (void)arena.allocate(16);
        throw std::runtime_error("boom");
    } catch (const std::runtime_error&) {
        // expected
    }

    EXPECT_EQ(arena.used(), before);
    EXPECT_EQ(arena.frameDepth(), 0u);
}

// Verifies bytes rolled back when a scope exits are available for reuse.
TEST(ScopeRollback, AllocationReusedAfterScopeExit) {
    Arena<> arena(64);
    std::byte* first = nullptr;
    {
        ArenaScope<false> scope(arena);
        first = arena.allocate(32);
    }
    std::byte* second = arena.allocate(32);
    EXPECT_EQ(first, second);
}
