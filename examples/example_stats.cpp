// Example Stats
//
// Covers:
// - Initial statistics after construction
// - Statistics after allocation
// - Peak watermark tracking
// - Statistics after endFrame (lifetime counters preserved)
// - Statistics after reset (full clear)

#include "example_helper.h"

using namespace AllocatorPro;

int main() {
    mainTitle("\nStatistics Examples");
    borderLine();

    Arena<true> arena{4096};

    // Displays the initial statistics of a newly constructed arena.
    setTitle("Initial Statistics");

    const auto& s = arena.getStats();

    std::cout << "Total allocated : " << s.totalAllocated_ << "\n";
    std::cout << "Current used    : " << s.currentUsed_    << "\n";
    std::cout << "Peak used       : " << s.peakUsed_       << "\n";
    std::cout << "Allocations     : " << s.allocations_    << "\n\n";

    // Shows how statistics change after several allocations.
    setTitle("After Allocation");

    (void)arena.allocate(128, alignof(std::max_align_t));
    (void)arena.allocate(256, alignof(std::max_align_t));
    (void)arena.allocate(64,  alignof(std::max_align_t));

    std::cout << "Total allocated : " << s.totalAllocated_ << "\n";
    std::cout << "Current used    : " << s.currentUsed_    << "\n";
    std::cout << "Peak used       : " << s.peakUsed_       << "\n";
    std::cout << "Allocations     : " << s.allocations_    << "\n\n";

    // Demonstrates that the peak watermark records the highest usage reached.
    setTitle("Peak Watermark");

    const std::size_t peakBefore = s.peakUsed_;

    arena.beginFrame();
    (void)arena.allocate(1024, alignof(std::max_align_t));
    arena.endFrame();

    std::cout << "Peak before frame   : " << peakBefore    << "\n";
    std::cout << "Peak after endFrame : " << s.peakUsed_   << "\n";
    std::cout << "Current used        : " << s.currentUsed_ << "\n\n";

    // Shows that endFrame restores memory usage without resetting lifetime statistics.
    setTitle("After EndFrame");

    const std::size_t allocsBefore = s.allocations_;
    const std::size_t totalBefore  = s.totalAllocated_;

    arena.beginFrame();
    (void)arena.allocate(512, alignof(std::max_align_t));
    arena.endFrame();

    std::cout << "Allocations before  : " << allocsBefore       << "\n";
    std::cout << "Allocations after   : " << s.allocations_     << "\n";
    std::cout << "Total before        : " << totalBefore         << "\n";
    std::cout << "Total after         : " << s.totalAllocated_   << "\n";
    std::cout << "Current used        : " << s.currentUsed_      << "\n\n";

    // Demonstrates that reset clears both arena memory and accumulated statistics.
    setTitle("After Reset");

    std::cout << "Total allocated before : " << s.totalAllocated_ << "\n";
    std::cout << "Allocations before     : " << s.allocations_    << "\n";
    std::cout << "Peak before            : " << s.peakUsed_       << "\n";

    arena.reset();

    std::cout << "Total allocated after  : " << s.totalAllocated_ << "\n";
    std::cout << "Current used after     : " << s.currentUsed_    << "\n";
    std::cout << "Peak after             : " << s.peakUsed_       << "\n";
    std::cout << "Allocations after      : " << s.allocations_    << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}