// Running out of arena space.
//
// Demonstrates:
// - allocate() returning nullptr instead of throwing when space runs out
// - The importance of checking the return value
// - create<T>() propagating that same nullptr on allocation failure

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("Exhausting A Small Arena");

    Arena<> arena(16);

    std::byte* first = arena.allocate(10);
    std::cout << "first allocate(10) : " << (first != nullptr ? "ok" : "failed") << "\n";

    std::byte* second = arena.allocate(10);
    std::cout << "second allocate(10): " << (second != nullptr ? "ok" : "failed") << "\n\n";

    // Unlike a throwing allocator, Arena never throws on exhaustion —
    // allocate() (and everything built on it) simply returns nullptr.
    // Code that doesn't check for that ends up dereferencing a null
    // pointer instead of getting a clean error to handle.
    setTitle("The Wrong Way");

    // int* bad = arena.allocate<int>();
    // *bad = 1; // UB: bad is nullptr, arena is already full

    setTitle("The Right Way");

    if (int* ok = arena.allocate<int>()) {
        *ok = 1;
    } else {
        std::cout << "allocation failed — handled without touching memory\n";
    }
}

REGISTER_EXAMPLE_SUITE();
