// Arena view() test suite.
//
// Coverage:
// - Span size matches used()
// - Span starts at the first allocated byte
// - A freshly constructed arena has a zero-size view

#include <support/framework.h>

using namespace ArenaPro;

// Verifies the returned span's size equals used().
static void span_size_matches_used() {
    Arena<> arena(64);
    (void)arena.allocate(10);
    CHK(arena.view().size() == arena.used());
}

// Verifies the returned span covers exactly the allocated bytes.
static void span_covers_allocated_bytes() {
    Arena<> arena(64);
    std::byte* p = arena.allocate(10);
    CHK(arena.view().data() == p);
}

// Verifies a fresh arena reports an empty view.
static void empty_arena_has_zero_size_view() {
    Arena<> arena(64);
    CHK(arena.view().size() == 0);
}

// Executes all view() test cases.
static void run_tests() {
    RUN(span_size_matches_used);
    RUN(span_covers_allocated_bytes);
    RUN(empty_arena_has_zero_size_view);
}

REGISTER_TEST_SUITE();
