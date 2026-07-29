// Exception safety of create<T>().
//
// Demonstrates:
// - What happens when a constructed type's constructor throws
// - The arena's storage and bookkeeping are unaffected by the exception
// - The reserved-but-unconstructed bytes are simply never reused

#include <support/framework.h>

using namespace ArenaPro;

namespace {

struct Fragile {
    explicit Fragile(bool should_throw) {
        if (should_throw) {
            throw std::runtime_error("Fragile construction failed");
        }
    }
};

} // namespace

static void run_examples() {

    setTitle("A Throwing Constructor");

    Arena<> arena(256);

    std::size_t used_before = arena.used();

    try {
        (void)arena.create<Fragile>(true);
    } catch (const std::exception& e) {
        std::cout << "caught: " << e.what() << "\n";
    }

    // allocate() already advanced the cursor before the constructor ran,
    // so the exception leaves used() where it was after the (successful)
    // allocation — the bytes reserved for the failed Fragile are simply
    // never reused, not returned to a pool of free space.
    setTitle("Arena State After The Throw");

    std::cout << "used before attempt: " << used_before << "\n";
    std::cout << "used after throw   : " << arena.used() << "\n\n";

    setTitle("A Succeeding Construction Still Works");

    Fragile* ok = arena.create<Fragile>(false);
    std::cout << "construction succeeded: " << (ok != nullptr) << "\n";
}

REGISTER_EXAMPLE_SUITE();
