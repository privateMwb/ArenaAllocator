// Arena allocate() test suite.
//
// Coverage:
// - Returns a non-null pointer when capacity allows
// - Returns nullptr when the request exceeds remaining capacity
// - Successive allocations advance the cursor
// - A zero-size request succeeds (allowed per contract)

#include <support/framework.h>

using namespace ArenaPro;

// Verifies a request that fits in the buffer succeeds.
static void returns_pointer_within_capacity() {
    Arena<> arena(64);
    std::byte* p = arena.allocate(16);
    CHK(p != nullptr);
}

// Verifies a request larger than the remaining capacity fails.
static void returns_nullptr_when_out_of_space() {
    Arena<> arena(8);
    std::byte* p = arena.allocate(16);
    CHK(p == nullptr);
}

// Verifies the bump-pointer cursor moves forward after each allocation.
static void successive_allocations_advance_cursor() {
    Arena<> arena(64);
    const std::size_t before = arena.used();

    (void)arena.allocate(8);
    const std::size_t afterFirst = arena.used();
    CHK(afterFirst > before);

    (void)arena.allocate(8);
    CHK(arena.used() > afterFirst);
}

// Verifies a zero-size request is accepted and returns a valid pointer.
static void zero_size_allocation_succeeds() {
    Arena<> arena(64);
    std::byte* p = arena.allocate(0);
    CHK(p != nullptr);
}

// Executes all allocate() test cases.
static void run_tests() {
    RUN(returns_pointer_within_capacity);
    RUN(returns_nullptr_when_out_of_space);
    RUN(successive_allocations_advance_cursor);
    RUN(zero_size_allocation_succeeds);
}

REGISTER_TEST_SUITE();
