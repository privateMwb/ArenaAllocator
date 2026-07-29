// Allocation statistics with Arena<true>.
//
// Demonstrates:
// - Enabling stats tracking via the EnableStats template parameter
// - totalAllocated_, currentUsed_, peakUsed_, and allocations_
// - peakUsed_ surviving a frame rollback that lowers currentUsed_

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("An Arena With Stats Enabled");

    Arena<true> arena(1024);

    (void)arena.allocate(100);
    (void)arena.allocate(200);
    (void)arena.allocate(50);

    const auto& stats = arena.getStats();
    std::cout << "totalAllocated: " << stats.totalAllocated_ << "\n";
    std::cout << "currentUsed   : " << stats.currentUsed_ << "\n";
    std::cout << "peakUsed      : " << stats.peakUsed_ << "\n";
    std::cout << "allocations   : " << stats.allocations_ << "\n\n";

    setTitle("Peak Survives A Rollback");

    arena.beginFrame();
    (void)arena.allocate(500);
    std::cout << "currentUsed at frame depth: " << arena.getStats().currentUsed_ << "\n";
    std::cout << "peakUsed at frame depth   : " << arena.getStats().peakUsed_ << "\n";

    arena.endFrame();

    // currentUsed_ drops back down with the rollback, but peakUsed_
    // records the high-water mark and is never lowered by endFrame().
    std::cout << "currentUsed after endFrame(): " << arena.getStats().currentUsed_ << "\n";
    std::cout << "peakUsed after endFrame()   : " << arena.getStats().peakUsed_ << "\n\n";

    setTitle("Reset Clears Everything");

    arena.reset();

    std::cout << "totalAllocated after reset(): " << arena.getStats().totalAllocated_ << "\n";
    std::cout << "peakUsed after reset()      : " << arena.getStats().peakUsed_ << "\n";
}

REGISTER_EXAMPLE_SUITE();
