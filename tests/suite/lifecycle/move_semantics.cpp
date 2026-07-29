// Arena move semantics test suite.
//
// Coverage:
// - Move construction transfers buffer/cursor/frame state
// - Move construction leaves the source valid and empty
// - Move assignment transfers state and releases the destination's own buffer
// - Move assignment leaves the source valid and empty
// - Self-move-assignment is a safe no-op

#include <support/framework.h>

#include <utility>

using namespace ArenaPro;

// Verifies the destination inherits the source's capacity, cursor, and frames.
static void move_construct_transfers_state() {
    Arena<> source(64);
    (void)source.allocate(10);
    source.beginFrame();

    Arena<> dest(std::move(source));
    CHK(dest.capacity() == 64);
    CHK(dest.used() == 10);
    CHK(dest.frameDepth() == 1);
}

// Verifies the moved-from source is left in a valid, empty state.
static void move_construct_leaves_source_empty() {
    Arena<> source(64);
    (void)source.allocate(10);

    Arena<> dest(std::move(source));
    (void)dest;

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(source.capacity() == 0);
    CHK(source.used() == 0);
    CHK(source.frameDepth() == 0);
}

// Verifies move assignment transfers the source's state into an
// already-constructed destination.
static void move_assign_transfers_state() {
    Arena<> source(64);
    (void)source.allocate(20);

    Arena<> dest(32);
    dest = std::move(source);

    CHK(dest.capacity() == 64);
    CHK(dest.used() == 20);
}

// Verifies the moved-from source is left in a valid, empty state after
// move assignment.
static void move_assign_leaves_source_empty() {
    Arena<> source(64);
    (void)source.allocate(20);

    Arena<> dest(32);
    dest = std::move(source);

    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(source.capacity() == 0);
    CHK(source.used() == 0);
}

// Verifies self-move-assignment does not corrupt or release the arena.
static void self_move_assignment_is_safe() {
    Arena<> arena(64);
    (void)arena.allocate(10);

    Arena<>& ref = arena;
    arena = std::move(ref);

    CHK(arena.capacity() == 64);
    CHK(arena.used() == 10);
}

// Executes all move semantics test cases.
static void run_tests() {
    RUN(move_construct_transfers_state);
    RUN(move_construct_leaves_source_empty);
    RUN(move_assign_transfers_state);
    RUN(move_assign_leaves_source_empty);
    RUN(self_move_assignment_is_safe);
}

REGISTER_TEST_SUITE();
