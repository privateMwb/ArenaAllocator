// Inspecting an arena's contents with view().
//
// Demonstrates:
// - view() returning a read-only span over everything allocated so far
// - The span growing as more is allocated, and shrinking on rollback
// - Reading raw bytes without any knowledge of what was stored there

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("An Empty View");

    Arena<> arena(64);

    std::span<const std::byte> v = arena.view();
    std::cout << "view size on an empty arena: " << v.size() << "\n\n";

    setTitle("View Grows With Allocation");

    (void)arena.allocate(10);
    (void)arena.allocate(20);

    v = arena.view();
    std::cout << "view size after 30 bytes allocated: " << v.size() << "\n\n";

    setTitle("View Shrinks On Rollback");

    arena.beginFrame();
    (void)arena.allocate(15);
    std::cout << "view size inside frame    : " << arena.view().size() << "\n";

    arena.endFrame();
    std::cout << "view size after endFrame(): " << arena.view().size() << "\n\n";

    // view() is read-only and just a window over the buffer — it carries
    // no type information, so the caller is responsible for knowing how
    // to interpret the bytes.
    setTitle("Reading A Raw Byte");

    std::cout << "first byte allocated: " << std::to_integer<int>(arena.view()[0]) << "\n";
}

REGISTER_EXAMPLE_SUITE();
