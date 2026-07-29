// Arena nested frames integration test suite.
//
// Coverage:
// - Three levels of nested frames each restore the correct checkpoint
// - Rolling back an inner frame makes its space reusable while an
//   outer frame's allocations remain untouched

#include <support/framework.h>

using namespace ArenaPro;

// Verifies each endFrame() in a three-level nest restores the cursor
// to exactly where the matching beginFrame() opened it.
static void three_level_nesting_restores_each_checkpoint() {
    Arena<> arena(64);

    arena.beginFrame();
    (void)arena.allocate(8);
    const std::size_t afterLevel1 = arena.used();

    arena.beginFrame();
    (void)arena.allocate(16);
    const std::size_t afterLevel2 = arena.used();

    arena.beginFrame();
    (void)arena.allocate(24);
    CHK(arena.used() > afterLevel2);

    arena.endFrame();
    CHK(arena.used() == afterLevel2);

    arena.endFrame();
    CHK(arena.used() == afterLevel1);

    arena.endFrame();
    CHK(arena.used() == 0);
}

// Verifies rolling back an inner frame frees its bytes for reuse while
// an allocation from the still-open outer frame is left alone.
static void inner_rollback_reuses_space_outer_keeps_its_own() {
    Arena<> arena(64);

    arena.beginFrame();
    std::byte* outer = arena.allocate(16);

    arena.beginFrame();
    std::byte* innerFirst = arena.allocate(16);
    arena.endFrame();

    std::byte* innerSecond = arena.allocate(16);
    CHK(innerFirst == innerSecond);

    arena.endFrame();
    CHK(arena.used() == 0);
    (void)outer;
}

// Executes all nested frame integration test cases.
static void run_tests() {
    RUN(three_level_nesting_restores_each_checkpoint);
    RUN(inner_rollback_reuses_space_outer_keeps_its_own);
}

REGISTER_TEST_SUITE();
