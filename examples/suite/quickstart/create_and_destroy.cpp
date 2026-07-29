// Object creation and destruction with Arena.
//
// Demonstrates:
// - create<T>() constructing several objects in sequence
// - destroy<T>() running a destructor without freeing storage
// - Why used() does not shrink after destroy()
// - Reclaiming storage in bulk via reset()

#include <support/framework.h>

using namespace ArenaPro;

namespace {

struct Widget {
    int id;
    explicit Widget(int id) : id{id} {
        std::cout << "Widget(" << id << ") constructed\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id << ") destroyed\n";
    }
};

} // namespace

static void run_examples() {

    setTitle("Creating Several Objects");

    Arena<> arena(1024);

    Widget* a = arena.create<Widget>(1);
    Widget* b = arena.create<Widget>(2);
    Widget* c = arena.create<Widget>(3);

    std::cout << "used after 3 creates: " << arena.used() << "\n\n";

    setTitle("Destroying One Object");

    // destroy() runs ~Widget(), but the bytes it occupied remain reserved —
    // the arena has no way to give back a single allocation.
    arena.destroy(b);

    std::cout << "used after destroy(b) (unchanged): " << arena.used() << "\n\n";

    setTitle("Reclaiming Storage In Bulk");

    // The only way to get that space back is bulk reclamation: reset()
    // rewinds everything at once. a and c must still be destroy()'d first —
    // reset() only rewinds bookkeeping, it does not run destructors.
    arena.destroy(a);
    arena.destroy(c);
    arena.reset();

    std::cout << "used after reset(): " << arena.used() << "\n";
}

REGISTER_EXAMPLE_SUITE();
