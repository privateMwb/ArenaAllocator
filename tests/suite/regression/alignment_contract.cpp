// Regression: requesting an alignment greater than the arena's base
// alignment is an AP_PRE (debug-only) contract violation by design —
// not a runtime-checked error. Documented on Arena::allocate() in
// Arena.h.
//
// Note: the violation itself can't be exercised here, since AP_PRE is
// a plain assert() and assert failure calls abort(), which isn't
// catchable via try/catch. This test instead pins the last *valid*
// boundary — requesting exactly the arena's base alignment — to guard
// against an off-by-one turning that boundary into a false failure
// (e.g. `<=` accidentally becoming `<`).

#include <cstdint>
#include <support/framework.h>

using namespace ArenaPro;

// Verifies requesting an alignment exactly equal to the arena's base
// alignment succeeds and produces a correctly aligned pointer.
static void alignment_equal_to_base_is_valid() {
    Arena<> arena(64, 32);
    std::byte* p = arena.allocate(1, 32);
    CHK(p != nullptr);
    CHK(reinterpret_cast<std::uintptr_t>(p) % 32 == 0);
}

// Executes all test cases for this regression.
static void run_tests() {
    RUN(alignment_equal_to_base_is_valid);
}

REGISTER_TEST_SUITE();
