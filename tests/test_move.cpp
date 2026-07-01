// Arena Move Test Suite
// Validates deeper move-semantics edge cases beyond basic ownership
// transfer, covering frame state, statistics, and memory replacement.
//
// Covers:
// - move construction with active frames open
// - move assignment with active frames open
// - move construction/assignment with statistics enabled
// - move-assigning into an arena that already owns memory
// - moving an empty (never-allocated) arena

#include "test_helper.h"

#include <cstddef>

// Verifies move construction preserves frame state and allocation progress.
static void move_construction_with_frames() {
    AllocatorPro::Arena<false> a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    a.beginFrame();
    (void)a.allocate(32, alignof(std::max_align_t));
    a.beginFrame();
    (void)a.allocate(16, alignof(std::max_align_t));

    CHK(a.frameDepth() == 2);
    CHK(a.used()       == 112);

    AllocatorPro::Arena<false> b{std::move(a)};

    CHK(b.frameDepth() == 2);
    CHK(b.used()       == 112);

    b.endFrame();
    CHK(b.used() == 96);

    b.endFrame();
    CHK(b.used() == 64);
}

// Verifies move assignment preserves frame state and allocation progress.
static void move_assignment_with_frames() {
    AllocatorPro::Arena<false> a{512};
    a.beginFrame();
    (void)a.allocate(48, alignof(std::max_align_t));

    AllocatorPro::Arena<false> b{128};
    b = std::move(a);

    CHK(b.frameDepth() == 1);
    CHK(b.used()       == 48);

    b.endFrame();
    CHK(b.used() == 0);
}

// Verifies move construction transfers allocation statistics to the destination.
static void move_construction_with_stats() {
    AllocatorPro::Arena<true> a{512};
    (void)a.allocate(64, alignof(std::max_align_t));
    (void)a.allocate(32, alignof(std::max_align_t));

    const auto& as = a.getStats();
    CHK(as.totalAllocated_ == 96);
    CHK(as.allocations_    == 2);

    AllocatorPro::Arena<true> b{std::move(a)};
    const auto& bs = b.getStats();

    CHK(bs.totalAllocated_ == 96);
    CHK(bs.allocations_    == 2);
    CHK(bs.currentUsed_    == 96);
}

// Verifies move assignment replaces the destination's allocation statistics.
static void move_assignment_with_stats() {
    AllocatorPro::Arena<true> a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    AllocatorPro::Arena<true> b{128};
    (void)b.allocate(8, alignof(std::max_align_t));

    b = std::move(a);
    const auto& bs = b.getStats();

    CHK(bs.totalAllocated_ == 64);
    CHK(bs.allocations_    == 1);
    CHK(bs.currentUsed_    == 64);
}

// Verifies move assignment replaces the destination's owned memory buffer.
static void move_assignment_replaces_memory() {
    AllocatorPro::Arena<false> a{512};
    (void)a.allocate(64, alignof(std::max_align_t));

    AllocatorPro::Arena<false> b{128};
    (void)b.allocate(16, alignof(std::max_align_t));

    void* aRaw = a.allocate(0, alignof(std::max_align_t));
    (void)aRaw;

    b = std::move(a);

    CHK(b.capacity() == 512);
    CHK(b.used()     == 64);
    CHK(a.capacity() == 0);
    CHK(a.used()     == 0);

    void* p = b.allocate(8, alignof(std::max_align_t));
    CHK(b.owns(p));
}

// Verifies moving an empty arena leaves both objects in a valid state.
static void move_empty_arena() {
    AllocatorPro::Arena<false> a{256};

    AllocatorPro::Arena<false> b{std::move(a)};

    CHK(b.capacity()  == 256);
    CHK(b.used()      == 0);
    CHK(b.remaining() == 256);

    CHK(a.capacity()  == 0);
    CHK(a.used()      == 0);
    CHK(a.remaining() == 0);

    void* p = b.allocate(32, alignof(std::max_align_t));
    CHK(p != nullptr);
    CHK(b.owns(p));
}

// Executes all move semantics test cases.
void run_move_tests() {
    setTitle("Move Tests");

    RUN(move_construction_with_frames);
    RUN(move_assignment_with_frames);
    RUN(move_construction_with_stats);
    RUN(move_assignment_with_stats);
    RUN(move_assignment_replaces_memory);
    RUN(move_empty_arena);

    std::cout << "\n";
}