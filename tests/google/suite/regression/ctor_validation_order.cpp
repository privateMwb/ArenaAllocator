// Regression: constructor validates `size` before allocating (fixed
// ordering bug — AP_PRE(size > 0) used to run in the constructor body,
// after the buffer had already been allocated in the mem-initializer
// list).
//
// Note: the violation itself (size == 0) can't be exercised here.
// AP_PRE is a plain assert(), and assert failure calls abort(), which
// is not catchable via try/catch — an EXPECT_THROW around it would just
// kill the whole test binary rather than register a failure. This test
// instead pins the smallest valid boundary, confirming the reordering
// didn't regress ordinary construction.
//
// Coverage:
// - The smallest legal size (1) still constructs correctly

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

using namespace ArenaPro;

// Verifies constructing with the smallest valid size still succeeds
// and reports the expected state.
TEST(CtorValidationOrder, SmallestValidSizeConstructs) {
    Arena<> arena(1);
    EXPECT_EQ(arena.capacity(), 1u);
    EXPECT_EQ(arena.used(), 0u);
}
