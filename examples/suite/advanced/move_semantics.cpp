// Move semantics on Arena.
//
// Demonstrates:
// - Move construction transferring the owned buffer
// - Move assignment releasing the destination's old buffer first
// - What's actually safe to call on a moved-from arena

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("Move Construction");

    Arena<> source(256);
    (void)source.allocate(64);
    std::cout << "source used before move: " << source.used() << "\n";

    Arena<> dest{std::move(source)};
    std::cout << "dest used after move    : " << dest.used() << "\n";
    std::cout << "dest capacity after move: " << dest.capacity() << "\n\n";

    // A moved-from arena is left valid but empty — zero capacity, zero
    // used. It's still safe to call any member function on it; there's
    // just nothing left to allocate from.
    setTitle("Moved-From State");

    std::cout << "source capacity after move: " << source.capacity() << "\n";
    std::cout << "source used after move    : " << source.used() << "\n";
    std::cout << "source allocate(1) result : "
              << (source.allocate(1) == nullptr ? "nullptr" : "non-null") << "\n\n";

    setTitle("Move Assignment");

    Arena<> a(128);
    Arena<> b(64);
    (void)a.allocate(32);
    (void)b.allocate(16);

    // Move-assigning releases a's current buffer before taking b's.
    a = std::move(b);

    std::cout << "a capacity after move-assign: " << a.capacity() << "\n";
    std::cout << "a used after move-assign    : " << a.used() << "\n";
    std::cout << "b capacity after move-assign: " << b.capacity() << "\n";
}

REGISTER_EXAMPLE_SUITE();
