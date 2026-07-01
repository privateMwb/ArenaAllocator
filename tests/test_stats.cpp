// Arena Statistics Test Suite
// Validates statistics tracking semantics, distinguishing lifetime
// counters from point-in-time usage across allocations, frames,
// and reset operations.
//
// Covers:
// - total allocated tracking
// - allocation counting
// - live usage tracking
// - peak usage tracking
// - frame rollback statistics
// - reset behavior
// - failed allocations

#include "test_helper.h"

// Verifies totalAllocated_ accumulates every successful allocation.
static void total_allocated_accumulates() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(64,  alignof(std::max_align_t));
    (void)arena.allocate(128, alignof(std::max_align_t));
    (void)arena.allocate(32,  alignof(std::max_align_t));

    CHK(arena.getStats().totalAllocated_ == 224);
}

// Verifies allocations_ increments once per successful allocation.
static void allocations_count() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(16, alignof(std::max_align_t));
    (void)arena.allocate(16, alignof(std::max_align_t));
    (void)arena.allocate(16, alignof(std::max_align_t));

    CHK(arena.getStats().allocations_ == 3);
}

// Verifies currentUsed_ always matches the arena's live usage.
static void current_used_tracks_live_usage() {
    AllocatorPro::Arena<true> arena{1024};

    CHK(arena.getStats().currentUsed_ == arena.used());

    (void)arena.allocate(100, alignof(std::max_align_t));
    CHK(arena.getStats().currentUsed_ == arena.used());
}

// Verifies peakUsed_ records the highest usage reached.
static void peak_used_tracks_watermark() {
    AllocatorPro::Arena<true> arena{1024};

    (void)arena.allocate(512, alignof(std::max_align_t));
    const std::size_t peak = arena.getStats().peakUsed_;

    arena.beginFrame();
    (void)arena.allocate(64, alignof(std::max_align_t));
    arena.endFrame();

    CHK(arena.getStats().peakUsed_ > peak);
    CHK(arena.getStats().currentUsed_ < arena.getStats().peakUsed_);
}

// Verifies endFrame updates only the current usage statistics.
static void end_frame_preserves_lifetime_counters() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(64, alignof(std::max_align_t));

    arena.beginFrame();
    (void)arena.allocate(128, alignof(std::max_align_t));
    (void)arena.allocate(128, alignof(std::max_align_t));

    const std::size_t allocsBeforeEnd = arena.getStats().allocations_;
    const std::size_t totalBeforeEnd  = arena.getStats().totalAllocated_;
    const std::size_t peakBeforeEnd   = arena.getStats().peakUsed_;

    arena.endFrame();

    CHK(arena.getStats().allocations_    == allocsBeforeEnd);
    CHK(arena.getStats().totalAllocated_ == totalBeforeEnd);
    CHK(arena.getStats().peakUsed_       == peakBeforeEnd);
    CHK(arena.getStats().currentUsed_    == arena.used());
}

// Verifies reset clears every statistics field.
static void reset_clears_all_statistics() {
    AllocatorPro::Arena<true> arena{1024};
    (void)arena.allocate(64,  alignof(std::max_align_t));
    (void)arena.allocate(128, alignof(std::max_align_t));

    arena.reset();

    const auto& s = arena.getStats();
    CHK(s.totalAllocated_ == 0);
    CHK(s.currentUsed_    == 0);
    CHK(s.peakUsed_       == 0);
    CHK(s.allocations_    == 0);
}

// Verifies failed allocations leave statistics unchanged.
static void failed_allocation_does_not_affect_stats() {
    AllocatorPro::Arena<true> arena{64};
    (void)arena.allocate(32, alignof(std::max_align_t));

    const std::size_t allocsBefore = arena.getStats().allocations_;
    const std::size_t totalBefore  = arena.getStats().totalAllocated_;

    void* p = arena.allocate(128, alignof(std::max_align_t));
    CHK(p == nullptr);

    CHK(arena.getStats().allocations_    == allocsBefore);
    CHK(arena.getStats().totalAllocated_ == totalBefore);
}

// Executes all statistics test cases.
void run_stats_tests() {
    setTitle("Statistics Tests");

    RUN(total_allocated_accumulates);
    RUN(allocations_count);
    RUN(current_used_tracks_live_usage);
    RUN(peak_used_tracks_watermark);
    RUN(end_frame_preserves_lifetime_counters);
    RUN(reset_clears_all_statistics);
    RUN(failed_allocation_does_not_affect_stats);

    std::cout << "\n";
}