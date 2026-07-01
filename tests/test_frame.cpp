// Arena Frame Management Test Suite
// Validates frame creation, rollback behavior, nested frames,
// automatic scope rollback, and statistics updates.
//
// Covers:
// - frame creation
// - frame rollback
// - nested frames
// - allocation rollback
// - statistics after rollback
// - ArenaScope rollback
// - nested ArenaScope rollback

#include "test_helper.h"

// Verifies beginFrame records the current allocation state.
static void begin_frame_saves_offset() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();
    arena.beginFrame();
    (void)arena.allocate(32, alignof(std::max_align_t));

    CHK(arena.used() > before);
}

// Verifies endFrame restores the previous allocation state.
static void end_frame_restores_offset() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();

    arena.beginFrame();
    (void)arena.allocate(128, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.used() == before);
}

// Verifies nested frames unwind in reverse order.
static void nested_frames() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t base = arena.used();

    arena.beginFrame();
    (void)arena.allocate(64, alignof(std::max_align_t));
    const std::size_t mid = arena.used();

    arena.beginFrame();
    (void)arena.allocate(64, alignof(std::max_align_t));

    arena.endFrame();
    CHK(arena.used() == mid);

    arena.endFrame();
    CHK(arena.used() == base);
}

// Verifies allocations inside a frame are discarded after rollback.
static void frame_discards_allocations() {
    AllocatorPro::Arena<false> arena{1024};

    arena.beginFrame();
    (void)arena.allocate(256, alignof(std::max_align_t));
    (void)arena.allocate(256, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.used()      == 0);
    CHK(arena.remaining() == 1024);
}

// Verifies rollback updates the current usage statistics.
static void stats_after_end_frame() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();

    arena.beginFrame();
    (void)arena.allocate(128, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.getStats().currentUsed_ == before);
}

// Verifies ArenaScope automatically restores the previous frame.
static void scope_restores_frame() {
    AllocatorPro::Arena<false> arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();

    {
        AllocatorPro::ArenaScope<false> scope{arena};
        (void)arena.allocate(256, alignof(std::max_align_t));
        CHK(arena.used() > before);
    }

    CHK(arena.used() == before);
}

// Verifies nested ArenaScope objects unwind in reverse order.
static void scope_nested() {
    AllocatorPro::Arena<false> arena{1024};

    const std::size_t base = arena.used();

    {
        AllocatorPro::ArenaScope<false> outer{arena};
        (void)arena.allocate(128, alignof(std::max_align_t));
        const std::size_t mid = arena.used();

        {
            AllocatorPro::ArenaScope<false> inner{arena};
            (void)arena.allocate(128, alignof(std::max_align_t));
            CHK(arena.used() > mid);
        }

        CHK(arena.used() == mid);
    }

    CHK(arena.used() == base);
}

// Executes all frame management test cases.
void run_frame_tests() {
    setTitle("Frame Management Tests");

    RUN(begin_frame_saves_offset);
    RUN(end_frame_restores_offset);
    RUN(nested_frames);
    RUN(frame_discards_allocations);
    RUN(stats_after_end_frame);
    RUN(scope_restores_frame);
    RUN(scope_nested);

    std::cout << "\n";
}