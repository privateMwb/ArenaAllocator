// Arena Frame Management Test Suite
// Validates frame push/pop behavior, nested frames, and state restoration
// after endFrame across single and multiple frame depths.
//
// Covers:
// - beginFrame saves current offset
// - endFrame restores offset after allocation
// - nested frames restore correctly in reverse order
// - allocations within frame are discarded after endFrame
// - stats reflect restored offset after endFrame
// - ArenaScope restores frame automatically on destruction

#include "test_helper.h"

// Begin Frame Saves Offset
// verifies beginFrame records the current offset onto the frame stack
static void begin_frame_saves_offset() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();
    arena.beginFrame();
    (void)arena.allocate(32, alignof(std::max_align_t));

    CHK(arena.used() > before);
}

// End Frame Restores Offset
// verifies endFrame rewinds the arena to the offset saved by beginFrame
static void end_frame_restores_offset() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();

    arena.beginFrame();
    (void)arena.allocate(128, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.used() == before);
}

// Nested Frames
// verifies nested beginFrame/endFrame pairs restore offsets in reverse order
static void nested_frames() {
    AllocatorPro::Arena arena{1024};

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

// Frame Discards Allocations
// verifies allocations made inside a frame are not visible after endFrame
static void frame_discards_allocations() {
    AllocatorPro::Arena arena{1024};

    arena.beginFrame();
    (void)arena.allocate(256, alignof(std::max_align_t));
    (void)arena.allocate(256, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.used()      == 0);
    CHK(arena.remaining() == 1024);
}

// Stats After End Frame
// verifies currentUsed_ reflects restored offset after endFrame
static void stats_after_end_frame() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();

    arena.beginFrame();
    (void)arena.allocate(128, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.getStats().currentUsed_ == before);
}

// Scope Restores Frame
// verifies ArenaScope automatically calls endFrame on destruction
static void scope_restores_frame() {
    AllocatorPro::Arena arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    const std::size_t before = arena.used();

    {
        AllocatorPro::ArenaScope scope{arena};
        (void)arena.allocate(256, alignof(std::max_align_t));
        CHK(arena.used() > before);
    }

    CHK(arena.used() == before);
}

// Scope Nested
// verifies nested ArenaScope instances restore offsets in correct order
static void scope_nested() {
    AllocatorPro::Arena arena{1024};

    const std::size_t base = arena.used();

    {
        AllocatorPro::ArenaScope outer{arena};
        (void)arena.allocate(128, alignof(std::max_align_t));
        const std::size_t mid = arena.used();

        {
            AllocatorPro::ArenaScope inner{arena};
            (void)arena.allocate(128, alignof(std::max_align_t));
            CHK(arena.used() > mid);
        }

        CHK(arena.used() == mid);
    }

    CHK(arena.used() == base);
}

// Test Runner
// Executes all frame management test cases.
void run_frame_management_tests() {
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