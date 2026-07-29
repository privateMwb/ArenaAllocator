// Arena create/destroy cycle integration test suite.
//
// Coverage:
// - destroy() runs the destructor but does not reclaim storage;
//   only reset() actually frees the bytes
// - An object created after reset() reuses the bytes an earlier,
//   destroyed object occupied

#include <support/framework.h>

using namespace ArenaPro;

namespace {

struct Widget {
    bool* destroyed;
    explicit Widget(bool* flag) : destroyed(flag) {}
    ~Widget() {
        *destroyed = true;
    }
};

} // namespace

// Verifies destroy() alone leaves used() unchanged, while reset() reclaims it.
static void destroy_does_not_free_space_but_reset_does() {
    Arena<> arena(64);
    bool destroyed = false;

    Widget* w = arena.create<Widget>(&destroyed);
    const std::size_t usedAfterCreate = arena.used();

    arena.destroy(w);
    CHK(destroyed);
    CHK(arena.used() == usedAfterCreate);

    arena.reset();
    CHK(arena.used() == 0);
}

// Verifies an object created after reset() lands on the same bytes an
// earlier, destroyed object used.
static void object_created_after_reset_reuses_freed_bytes() {
    Arena<> arena(64);
    bool destroyed = false;

    Widget* first = arena.create<Widget>(&destroyed);
    arena.destroy(first);
    arena.reset();

    bool destroyedAgain = false;
    Widget* second = arena.create<Widget>(&destroyedAgain);

    CHK(reinterpret_cast<std::byte*>(first) == reinterpret_cast<std::byte*>(second));
}

// Executes all create/destroy cycle integration test cases.
static void run_tests() {
    RUN(destroy_does_not_free_space_but_reset_does);
    RUN(object_created_after_reset_reuses_freed_bytes);
}

REGISTER_TEST_SUITE();
