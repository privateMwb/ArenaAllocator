// example_basic.cpp
// Demonstrates basic arena construction, raw allocation,
// typed allocation, reset, and introspection.

#include "example_helper.h"

#include <cstddef>

using namespace AllocatorPro;

int main() {
    mainTitle("\nBasic  Examples");
    borderLine();
    
    // Construction
    setTitle("Construction");

    Arena arena{1024};

    std::cout << "Capacity  : " << arena.capacity()  << "\n";
    std::cout << "Used      : " << arena.used()      << "\n";
    std::cout << "Remaining : " << arena.remaining() << "\n\n";

    // Raw Allocation
    setTitle("Raw Allocation");

    void* p1 = arena.allocate(64, alignof(std::max_align_t));
    void* p2 = arena.allocate(128, alignof(std::max_align_t));
    (void)p1;
    (void)p2;

    std::cout << "Used after 2 allocations      : " << arena.used()      << "\n";
    std::cout << "Remaining after 2 allocations : " << arena.remaining() << "\n\n";

    // Typed Allocation
    setTitle("Typed Allocation");

    int*    pi = arena.allocate<int>();
    double* pd = arena.allocate<double>();
    (void)pi;
    (void)pd;

    std::cout << "Used after typed allocations      : " << arena.used()      << "\n";
    std::cout << "Remaining after typed allocations : " << arena.remaining() << "\n\n";

    // Stats
    setTitle("Stats");

    const auto& s = arena.getStats();

    std::cout << "Total allocated : " << s.totalAllocated_ << "\n";
    std::cout << "Current used    : " << s.currentUsed_    << "\n";
    std::cout << "Peak used       : " << s.peakUsed_       << "\n";
    std::cout << "Allocations     : " << s.allocations_    << "\n\n";

    // Introspection
    setTitle("Introspection");

    std::cout << "Owns p1   : " << arena.owns(p1)      << "\n";
    std::cout << "Owns p2   : " << arena.owns(p2)      << "\n";
    std::cout << "View size : " << arena.view().size() << "\n\n";

    // Reset
    setTitle("Reset");

    std::cout << "Used before reset     : " << arena.used()     << "\n";
    std::cout << "Capacity before reset : " << arena.capacity() << "\n";

    arena.reset();

    std::cout << "Used after reset      : " << arena.used()     << "\n";
    std::cout << "Capacity after reset  : " << arena.capacity() << "\n";
    
    borderLine();
    std::cout << "\n";
    return 0;
}

