// Arena destroy() test suite.
//
// Coverage:
// - Runs the object's destructor
// - Does not return the storage to the arena

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

// Verifies destroy() runs T's destructor.
static void runs_destructor() {
    Arena<> arena(64);
    bool destroyed = false;
    Widget* w = arena.create<Widget>(&destroyed);
    CHK(w != nullptr);

    arena.destroy(w);
    CHK(destroyed);
}

// Verifies destroy() leaves the allocation cursor untouched.
static void does_not_reclaim_storage() {
    Arena<> arena(64);
    bool destroyed = false;
    Widget* w = arena.create<Widget>(&destroyed);
    const std::size_t usedBefore = arena.used();

    arena.destroy(w);
    CHK(arena.used() == usedBefore);
}

// Executes all destroy() test cases.
static void run_tests() {
    RUN(runs_destructor);
    RUN(does_not_reclaim_storage);
}

REGISTER_TEST_SUITE();
