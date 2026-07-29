// Frame-scoped allocation with Arena and ArenaScope.
//
// Demonstrates:
// - beginFrame()/endFrame() as a manual rollback checkpoint
// - ArenaScope managing the same checkpoint via RAII
// - Allocations made inside a frame disappearing when it closes

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("Manual Frames");

    Arena<> arena(1024);

    arena.beginFrame();
    (void)arena.allocate(200);
    std::cout << "used inside frame     : " << arena.used() << "\n";

    arena.endFrame();
    std::cout << "used after endFrame() : " << arena.used() << "\n\n";

    setTitle("ArenaScope (RAII)");

    // ArenaScope opens a frame on construction and closes it on
    // destruction, so the rollback happens even if the scope is left
    // early or via an exception.
    {
        ArenaScope<false> scope(arena);
        (void)arena.allocate(300);
        std::cout << "used inside scope    : " << arena.used() << "\n";
    }

    std::cout << "used after scope ends: " << arena.used() << "\n\n";

    setTitle("Frame Depth");

    std::cout << "frameDepth() at rest             : " << arena.frameDepth() << "\n";

    {
        ArenaScope<false> outer(arena);
        {
            ArenaScope<false> inner(arena);
            std::cout << "frameDepth() with 2 open scopes: " << arena.frameDepth() << "\n";
        }
        std::cout << "frameDepth() with 1 open scope  : " << arena.frameDepth() << "\n";
    }

    std::cout << "frameDepth() after both close     : " << arena.frameDepth() << "\n";
}

REGISTER_EXAMPLE_SUITE();
