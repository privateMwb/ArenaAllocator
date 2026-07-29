// Arena allocate/reset/reuse integration test suite.
//
// Coverage:
// - Filling the arena to capacity, then reset(), makes the full
//   capacity available again
// - The first allocation after reset() lands at the same address as
//   the very first allocation ever made

#include <support/framework.h>

using namespace ArenaPro;

// Verifies reset() restores full capacity after the arena was filled.
static void fill_to_capacity_then_reset_allows_reuse() {
    Arena<> arena(32);
    std::byte* p = arena.allocate(32);
    CHK(p != nullptr);
    CHK(arena.remaining() == 0);

    arena.reset();
    CHK(arena.remaining() == 32);

    std::byte* q = arena.allocate(32);
    CHK(q != nullptr);
}

// Verifies the cursor restarts from the beginning of the buffer after reset().
static void allocation_after_reset_starts_at_beginning() {
    Arena<> arena(64);
    std::byte* first = arena.allocate(16);

    arena.reset();
    std::byte* afterReset = arena.allocate(16);

    CHK(first == afterReset);
}

// Executes all allocate/reset/reuse integration test cases.
static void run_tests() {
    RUN(fill_to_capacity_then_reset_allows_reuse);
    RUN(allocation_after_reset_starts_at_beginning);
}

REGISTER_TEST_SUITE();
