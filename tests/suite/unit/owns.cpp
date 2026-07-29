// Arena owns() test suite.
//
// Coverage:
// - Pointer to a live allocation is reported as owned
// - Pointer outside the buffer is reported as not owned
// - One-past-the-end pointer is reported as not owned (half-open range)
// - nullptr is reported as not owned

#include <support/framework.h>

using namespace ArenaPro;

// Verifies a pointer returned by allocate() is reported as owned.
static void live_allocation_is_owned() {
    Arena<> arena(64);
    std::byte* p = arena.allocate(16);
    CHK(arena.owns(p));
}

// Verifies a pointer from an unrelated buffer is reported as not owned.
static void foreign_pointer_is_not_owned() {
    Arena<> arena(64);
    std::byte other[16];
    CHK(!arena.owns(other));
}

// Verifies the one-past-the-end address is excluded (range is half-open).
static void end_pointer_is_not_owned() {
    Arena<> arena(64);
    std::byte* p = arena.allocate(64);
    CHK(arena.owns(p));
    CHK(!arena.owns(p + 64));
}

// Verifies nullptr is reported as not owned.
static void null_pointer_is_not_owned() {
    Arena<> arena(64);
    CHK(!arena.owns(nullptr));
}

// Executes all owns() test cases.
static void run_tests() {
    RUN(live_allocation_is_owned);
    RUN(foreign_pointer_is_not_owned);
    RUN(end_pointer_is_not_owned);
    RUN(null_pointer_is_not_owned);
}

REGISTER_TEST_SUITE();
