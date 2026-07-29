// Nested frames on Arena.
//
// Demonstrates:
// - Opening several frames at once, up to the arena's fixed depth limit
// - Each endFrame() rolling back only to its matching beginFrame()
// - Frame nesting composing correctly with allocations at each level

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("Three Nested Frames");

    Arena<> arena(1024);

    (void)arena.allocate(50);
    std::cout << "used at depth 0: " << arena.used() << "\n";

    arena.beginFrame();
    (void)arena.allocate(50);
    std::cout << "used at depth 1: " << arena.used() << "\n";

    arena.beginFrame();
    (void)arena.allocate(50);
    std::cout << "used at depth 2: " << arena.used() << "\n";

    arena.beginFrame();
    (void)arena.allocate(50);
    std::cout << "used at depth 3: " << arena.used() << "\n\n";

    setTitle("Closing Them One At A Time");

    arena.endFrame();
    std::cout << "used back at depth 2: " << arena.used() << "\n";

    arena.endFrame();
    std::cout << "used back at depth 1: " << arena.used() << "\n";

    arena.endFrame();
    std::cout << "used back at depth 0: " << arena.used() << "\n\n";

    // Only the 50 bytes allocated before any frame was opened remain.
    setTitle("Depth Limit");

    // Arena supports up to 8 simultaneously open frames. Each beginFrame()
    // that stays within that limit behaves exactly like the ones above,
    // however deeply it's nested.
    std::cout << "frameDepth() at rest: " << arena.frameDepth() << "\n";
}

REGISTER_EXAMPLE_SUITE();
