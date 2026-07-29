// Basic Arena usage.
//
// Demonstrates:
// - Constructing an arena with a fixed buffer size
// - Raw allocation with allocate()
// - Typed allocation with allocate<T>() and create<T>()
// - Destroying an object with destroy()
// - Capacity, used, and remaining checks
// - Resetting an arena

#include <support/framework.h>

using namespace ArenaPro;

namespace {

struct Point {
    int x;
    int y;
};

} // namespace

static void run_examples() {

    // An arena is constructed with a fixed buffer size — the total number
    // of bytes it will ever be able to hand out.
    setTitle("Construction");

    Arena<> arena(1024);

    std::cout << "capacity : " << arena.capacity() << "\n";
    std::cout << "used     : " << arena.used() << "\n";
    std::cout << "remaining: " << arena.remaining() << "\n\n";

    // allocate() hands back raw, uninitialized bytes carved out of the
    // buffer, or nullptr if the request can't be satisfied.
    setTitle("Raw Allocation");

    std::byte* block = arena.allocate(64);
    std::cout << "allocate(64) succeeded: " << (block != nullptr) << "\n";
    std::cout << "used after allocate   : " << arena.used() << "\n\n";

    // allocate<T>() reserves storage sized and aligned for T, but does not
    // construct anything in it.
    setTitle("Typed Allocation");

    int* raw_int = arena.allocate<int>();
    *raw_int = 42;
    std::cout << "raw_int value: " << *raw_int << "\n\n";

    // create<T>() reserves storage and constructs T in place, returning a
    // ready-to-use pointer.
    setTitle("Construction In Place");

    Point* p = arena.create<Point>(3, 4);
    std::cout << "point: (" << p->x << ", " << p->y << ")\n\n";

    // destroy() runs the object's destructor. The underlying bytes stay
    // reserved — the arena has no concept of freeing a single allocation.
    setTitle("Destruction");

    arena.destroy(p);
    std::cout << "used after destroy() (unchanged): " << arena.used() << "\n\n";

    // used() and remaining() move together; capacity() never changes.
    setTitle("Capacity and Usage");

    std::cout << "capacity : " << arena.capacity() << "\n";
    std::cout << "used     : " << arena.used() << "\n";
    std::cout << "remaining: " << arena.remaining() << "\n\n";

    // reset() rewinds the whole arena to empty in O(1), making every byte
    // available for reuse. It does not touch or zero the buffer itself.
    setTitle("Reset");

    arena.reset();

    std::cout << "used after reset()     : " << arena.used() << "\n";
    std::cout << "remaining after reset(): " << arena.remaining() << "\n";
}

REGISTER_EXAMPLE_SUITE();
