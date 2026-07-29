// Regression: constructor validates `size` before allocating (fixed
// ordering bug — AP_PRE(size > 0) used to run in the constructor body,
// after the buffer had already been allocated in the mem-initializer
// list).
//
// Note: the violation itself (size == 0) can't be exercised here.
// AP_PRE is a plain assert(), and assert failure calls abort(), which
// is not catchable via try/catch — a CHK_THROWS around it would just
// kill the whole test binary rather than register a [FAIL]. This test
// instead pins the smallest valid boundary, confirming the reordering
// didn't regress ordinary construction.
//
// Coverage:
// - The smallest legal size (1) still constructs correctly

#include <support/framework.h>

using namespace ArenaPro;

// Verifies constructing with the smallest valid size still succeeds
// and reports the expected state.
static void smallest_valid_size_constructs_successfully() {
    Arena<> arena(1);
    CHK(arena.capacity() == 1);
    CHK(arena.used() == 0);
}

// Executes all test cases for this regression.
static void run_tests() {
    RUN(smallest_valid_size_constructs_successfully);
}

REGISTER_TEST_SUITE();
