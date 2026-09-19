// Arena / ArenaScope integration test suite.
//
// Coverage:
// - ArenaScope opens a frame on construction, closes it on destruction
// - The frame is rolled back even when the scoped code throws
// - Storage freed by the rollback is handed out again

#include <support/framework.h>

#include <stdexcept>

using namespace ArenaPro;

// Verifies the scope's RAII frame tracks arena.frameDepth() correctly.
static void opens_and_closes_frame_via_raii() {
    Arena<> arena(64);
    CHK(arena.frameDepth() == 0);
    {
        ArenaScope<false> scope(arena);
        CHK(arena.frameDepth() == 1);
    }
    CHK(arena.frameDepth() == 0);
}

// Verifies an exception unwinding through a scope still rolls back its frame.
static void rolls_back_on_exception() {
    Arena<> arena(64);
    const std::size_t before = arena.used();

    try {
        ArenaScope<false> scope(arena);
        (void)arena.allocate(16);
        throw std::runtime_error("boom");
    } catch (const std::runtime_error&) {
        // expected
    }

    CHK(arena.used() == before);
    CHK(arena.frameDepth() == 0);
}

// Verifies bytes rolled back when a scope exits are available for reuse.
static void allocation_reused_after_scope_exit() {
    Arena<> arena(64);
    std::byte* first = nullptr;
    {
        ArenaScope<false> scope(arena);
        first = arena.allocate(32);
    }
    std::byte* second = arena.allocate(32);
    CHK(first == second);
}

// Executes all Arena/ArenaScope integration test cases.
static void run_tests() {
    RUN(opens_and_closes_frame_via_raii);
    RUN(rolls_back_on_exception);
    RUN(allocation_reused_after_scope_exit);
}

REGISTER_TEST_SUITE();
