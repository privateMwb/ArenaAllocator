// ArenaScope Test Suite
// Validates RAII-based frame management, ensuring frames are opened
// and closed automatically as scopes are entered and exited.
//
// Covers:
// - automatic frame creation
// - automatic frame restoration
// - sequential scope usage
// - allocation rollback
// - empty scope behavior
// - copy and move restrictions

#include "test_helper.h"

// Verifies constructing an ArenaScope opens a new frame.
static void scope_opens_frame_on_construction() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t before = arena.frameDepth();

    AllocatorPro::ArenaScope<false> scope{arena};

    CHK(arena.frameDepth() == before + 1);
}

// Verifies destroying an ArenaScope restores the previous frame.
static void scope_closes_frame_on_destruction() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t before = arena.frameDepth();

    {
        AllocatorPro::ArenaScope<false> scope{arena};
        CHK(arena.frameDepth() == before + 1);
    }

    CHK(arena.frameDepth() == before);
}

// Verifies sequential scopes restore the frame state independently.
static void sequential_scopes() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t base = arena.frameDepth();

    {
        AllocatorPro::ArenaScope<false> first{arena};
        (void)arena.allocate(64, alignof(std::max_align_t));
    }
    CHK(arena.frameDepth() == base);

    {
        AllocatorPro::ArenaScope<false> second{arena};
        (void)arena.allocate(64, alignof(std::max_align_t));
    }
    CHK(arena.frameDepth() == base);
}

// Verifies allocations made within a scope are discarded.
static void scope_discards_allocations() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t before = arena.used();

    {
        AllocatorPro::ArenaScope<false> scope{arena};
        (void)arena.allocate(512, alignof(std::max_align_t));
        CHK(arena.used() > before);
    }

    CHK(arena.used() == before);
}

// Verifies an empty scope leaves the arena unchanged.
static void scope_empty() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t before = arena.used();

    {
        AllocatorPro::ArenaScope<false> scope{arena};
    }

    CHK(arena.used() == before);
}

// Verifies ArenaScope is neither copyable nor movable.
static void scope_non_copyable_non_movable() {
    using Scope = AllocatorPro::ArenaScope<false>;

    static_assert(!std::is_copy_constructible_v<Scope>);
    static_assert(!std::is_copy_assignable_v<Scope>);
    static_assert(!std::is_move_constructible_v<Scope>);
    static_assert(!std::is_move_assignable_v<Scope>);

    CHK(true);
}

// Executes all ArenaScope test cases.
void run_scope_tests() {
    setTitle("ArenaScope Tests");

    RUN(scope_opens_frame_on_construction);
    RUN(scope_closes_frame_on_destruction);
    RUN(sequential_scopes);
    RUN(scope_discards_allocations);
    RUN(scope_empty);
    RUN(scope_non_copyable_non_movable);

    std::cout << "\n";
}