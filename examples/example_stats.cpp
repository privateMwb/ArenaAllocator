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

    dataFormat("Total allocated",  s.totalAllocated_);
    dataFormat("Current used",     s.currentUsed_);
    dataFormat("Peak used",        s.peakUsed_);
    dataFormat("Allocations",      s.allocations_);
    std::cout << "\n";

    // Shows how statistics change after several allocations.
    setTitle("After Allocation");

    (void)arena.allocate(128, alignof(std::max_align_t));
    (void)arena.allocate(256, alignof(std::max_align_t));
    (void)arena.allocate(64,  alignof(std::max_align_t));

    dataFormat("Total allocated",  s.totalAllocated_);
    dataFormat("Current used",     s.currentUsed_);
    dataFormat("Peak used",        s.peakUsed_);
    dataFormat("Allocations",      s.allocations_);
    std::cout << "\n";

    // Demonstrates that the peak watermark records the highest usage reached.
    setTitle("Peak Watermark");

    const std::size_t peakBefore = s.peakUsed_;

    arena.beginFrame();
    (void)arena.allocate(1024, alignof(std::max_align_t));
    arena.endFrame();

    dataFormat("Peak before frame",   peakBefore);
    dataFormat("Peak after endFrame", s.peakUsed_);
    dataFormat("Current used",        s.currentUsed_);
    std::cout << "\n";

    // Shows that endFrame restores memory usage without resetting lifetime statistics.
    setTitle("After EndFrame");

    const std::size_t allocsBefore = s.allocations_;
    const std::size_t totalBefore  = s.totalAllocated_;

    arena.beginFrame();
    (void)arena.allocate(512, alignof(std::max_align_t));
    arena.endFrame();

    dataFormat("Allocations before",  allocsBefore);
    dataFormat("Allocations after",   s.allocations_);
    dataFormat("Total before",        totalBefore);
    dataFormat("Total after",         s.totalAllocated_);
    dataFormat("Current used",        s.currentUsed_);
    std::cout << "\n";

    // Demonstrates that reset clears both arena memory and accumulated statistics.
    setTitle("After Reset");

    dataFormat("Total allocated before", s.totalAllocated_);
    dataFormat("Allocations before",     s.allocations_);
    dataFormat("Peak before",            s.peakUsed_);

    arena.reset();

    dataFormat("Total allocated after",  s.totalAllocated_);
    dataFormat("Current used after",     s.currentUsed_);
    dataFormat("Peak after",             s.peakUsed_);
    dataFormat("Allocations after",      s.allocations_);

    borderLine();
    std::cout << "\n";
    return 0;
}