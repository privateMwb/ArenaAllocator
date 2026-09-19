// Arena endFrame() test suite.
//
// Coverage:
// - Rewinds the allocation cursor to the matching beginFrame() checkpoint
// - Decrements the frame depth
// - Storage freed by the rollback is available for reuse

#include <support/framework.h>

using namespace ArenaPro;

// Verifies endFrame() restores the cursor to where beginFrame() opened it.
static void rewinds_cursor_to_checkpoint() {
    Arena<> arena(64);
    (void)arena.allocate(8);
    const std::size_t checkpoint = arena.used();

    arena.beginFrame();
    (void)arena.allocate(16);
    arena.endFrame();

    CHK(arena.used() == checkpoint);
}

// Verifies endFrame() decrements frameDepth() by one.
static void decrements_frame_depth() {
    Arena<> arena(64);
    arena.beginFrame();
    arena.beginFrame();

    arena.endFrame();
    CHK(arena.frameDepth() == 1);

    arena.endFrame();
    CHK(arena.frameDepth() == 0);
}

// Verifies bytes rolled back by endFrame() are handed out again.
static void reclaimed_space_is_reusable() {
    Arena<> arena(64);
    arena.beginFrame();
    std::byte* first = arena.allocate(32);
    arena.endFrame();

    std::byte* second = arena.allocate(32);
    CHK(first == second);
}

// Executes all endFrame() test cases.
static void run_tests() {
    RUN(rewinds_cursor_to_checkpoint);
    RUN(decrements_frame_depth);
    RUN(reclaimed_space_is_reusable);
}

REGISTER_TEST_SUITE();
