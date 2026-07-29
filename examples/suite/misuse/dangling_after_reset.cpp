// Dangling pointers after reset() or endFrame().
//
// Demonstrates:
// - A pointer returned by allocate()/create() becoming invalid once its
//   storage is rolled back
// - Why Arena can't detect this for you
// - The correct pattern: stop using pointers once their frame closes

#include <support/framework.h>

using namespace ArenaPro;

namespace {
struct Token {
    int value;
};
} // namespace

static void run_examples() {

    setTitle("A Pointer Into A Frame");

    Arena<> arena(256);

    arena.beginFrame();
    Token* t = arena.create<Token>(7);
    std::cout << "t->value while frame is open: " << t->value << "\n\n";

    setTitle("The Wrong Way");

    arena.endFrame();

    // t now points into bytes the arena considers free — the memory is
    // still physically there, but the arena is free to hand those same
    // bytes to the next allocate() call. Reading through t is UB from
    // this point on; not shown running for real:
    //
    //   std::cout << t->value; // UB: t's storage was rolled back

    setTitle("The Right Way");

    // Treat any pointer as dead the moment its owning frame closes —
    // scope the pointer's lifetime to match, ideally with ArenaScope so
    // the compiler enforces the block boundary.
    {
        ArenaScope<false> scope(arena);
        Token* fresh = arena.create<Token>(9);
        std::cout << "fresh->value while its scope is open: " << fresh->value << "\n";
    }
    // fresh is out of scope here too, in both the C++ and arena senses.
}

REGISTER_EXAMPLE_SUITE();
