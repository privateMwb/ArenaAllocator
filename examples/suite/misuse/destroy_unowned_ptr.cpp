// Calling destroy() on a pointer the arena doesn't own.
//
// Demonstrates:
// - destroy()'s precondition that the pointer belongs to this arena
// - owns() as the way to check that before destroying
// - The failure mode if the precondition is violated (shown, not
//   executed)

#include <support/framework.h>

using namespace ArenaPro;

namespace {
struct Tag {
    int id;
};
} // namespace

static void run_examples() {

    setTitle("A Pointer From Elsewhere");

    Arena<> arena(128);
    Tag stack_tag{99}; // lives on the stack, not in arena's buffer

    std::cout << "arena.owns(&stack_tag): " << arena.owns(&stack_tag) << "\n\n";

    setTitle("The Wrong Way");

    // destroy() asserts owns(ptr) as a precondition; it does not check
    // it at runtime in a release build. Calling it on a pointer the
    // arena never handed out is a contract violation, not a checked
    // error — shown here rather than run:
    //
    //   arena.destroy(&stack_tag); // AP_PRE(owns(ptr)) fails in debug,
    //                              // UB in release

    setTitle("The Right Way");

    Tag* arena_tag = arena.create<Tag>(1);

    if (arena.owns(arena_tag)) {
        arena.destroy(arena_tag);
        std::cout << "destroyed a pointer the arena actually owns\n";
    }
}

REGISTER_EXAMPLE_SUITE();
