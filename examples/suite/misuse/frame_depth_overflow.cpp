// Exceeding Arena's maximum frame depth.
//
// Demonstrates:
// - The fixed limit on nested beginFrame()/endFrame() checkpoints
// - Why the limit exists: frameStack_ is a fixed-size array, not a
//   dynamically growing stack
// - The failure mode if it's exceeded (shown, not executed)

#include <support/framework.h>

using namespace ArenaPro;

static void run_examples() {

    setTitle("Opening Up To The Limit");

    Arena<> arena(4096);

    // Arena supports at most 8 simultaneously open frames. Opening
    // exactly that many is well-defined:
    for (int i = 0; i < 8; ++i) {
        arena.beginFrame();
    }
    std::cout << "frameDepth() at the limit: " << arena.frameDepth() << "\n\n";

    setTitle("What Not To Do");

    // A 9th beginFrame() here has no bounds check outside of an assert —
    // frameStack_ is a fixed std::array<std::size_t, 8>, so this would
    // write past its end. In a debug build AP_PRE() catches it; in a
    // release build (NDEBUG) it's undefined behavior. This is why it's
    // shown here rather than actually run:
    //
    //   arena.beginFrame(); // frameDepth_ == 8, contract violation

    std::cout << "correct usage: check frameDepth() before opening another frame,\n"
              << "or keep nesting shallow enough to never approach the limit.\n\n";

    setTitle("Closing Back Down");

    for (int i = 0; i < 8; ++i) {
        arena.endFrame();
    }
    std::cout << "frameDepth() after closing all: " << arena.frameDepth() << "\n";
}

REGISTER_EXAMPLE_SUITE();
