// Basic Arena example.
//
// Covers:
// - Arena construction
// - Raw memory allocation
// - Aligned allocation
// - Frame management
// - Arena introspection
// - Arena reset

#include "example_helper.h"

using namespace AllocatorPro;

int main() {
    mainTitle("\nBasic Examples");
    borderLine();

    // Constructs an arena and displays its initial state.
    setTitle("Construction");

    Arena<false> arena{1024};

    std::cout << "Capacity  : " << arena.capacity()  << "\n";
    std::cout << "Used      : " << arena.used()      << "\n";
    std::cout << "Remaining : " << arena.remaining() << "\n\n";

    // Allocates raw memory blocks from the arena.
    setTitle("Raw Allocation");

    std::byte* p1 = arena.allocate(64, alignof(std::max_align_t));
    std::byte* p2 = arena.allocate(128, alignof(std::max_align_t));
    std::byte* p3 = arena.allocate(32, alignof(std::max_align_t));
    (void)p1;
    (void)p2;
    (void)p3;

    std::cout << "Used after 3 allocations      : " << arena.used()      << "\n";
    std::cout << "Remaining after 3 allocations : " << arena.remaining() << "\n\n";

    // Allocates memory using different alignment requirements.
    setTitle("Aligned Allocation");

    arena.reset();

    std::byte* a16 = arena.allocate(64, 16);
    std::byte* a32 = arena.allocate(64, 32);
    std::byte* a64 = arena.allocate(64, 64);
    (void)a16;
    (void)a32;
    (void)a64;

    std::cout << "16-byte aligned ptr : " << static_cast<void*>(a16) << "\n";
    std::cout << "32-byte aligned ptr : " << static_cast<void*>(a32) << "\n";
    std::cout << "64-byte aligned ptr : " << static_cast<void*>(a64) << "\n\n";

    // Demonstrates temporary allocations using stack-like frames.
    setTitle("Frame Management");

    arena.reset();

    std::byte* base = arena.allocate(64, alignof(std::max_align_t));
    (void)base;

    std::cout << "Used before frame   : " << arena.used()       << "\n";
    std::cout << "Frame depth before  : " << arena.frameDepth() << "\n";

    arena.beginFrame();

    std::byte* temp = arena.allocate(256, alignof(std::max_align_t));
    (void)temp;

    std::cout << "Used inside frame   : " << arena.used()       << "\n";
    std::cout << "Frame depth inside  : " << arena.frameDepth() << "\n";

    arena.endFrame();

    std::cout << "Used after endFrame : " << arena.used()       << "\n";
    std::cout << "Frame depth after   : " << arena.frameDepth() << "\n\n";

    // Queries arena ownership and memory view information.
    setTitle("Introspection");

    void* owned = arena.allocate(64, alignof(std::max_align_t));
    int x = 0;

    std::cout << "Owns allocated ptr : " << arena.owns(owned)    << "\n";
    std::cout << "Owns stack var &x  : " << arena.owns(&x)       << "\n";
    std::cout << "Owns nullptr       : " << arena.owns(nullptr)  << "\n";
    std::cout << "View size          : " << arena.view().size()  << "\n\n";

    // Resets the arena to reclaim all allocated memory.
    setTitle("Reset");

    std::cout << "Used before reset      : " << arena.used()      << "\n";
    std::cout << "Remaining before reset : " << arena.remaining() << "\n";

    arena.reset();

    std::cout << "Used after reset       : " << arena.used()      << "\n";
    std::cout << "Remaining after reset  : " << arena.remaining() << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}