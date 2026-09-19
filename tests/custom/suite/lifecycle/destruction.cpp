// Arena destruction test suite.
//
// Coverage:
// - Objects created via create() but never destroy()'d are not destructed
//   when the arena itself goes out of scope
// - A moved-from arena (null buffer) destructs safely

#include <support/framework.h>

#include <utility>

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

// Verifies the arena's destructor only releases the buffer — it does not
// run destructors for objects still live inside it.
static void live_objects_are_not_destructed() {
    bool destroyed = false;
    {
        Arena<> arena(64);
        (void)arena.create<Widget>(&destroyed);
    } // arena destructs here
    CHK(!destroyed);
}

// Verifies destructing a moved-from arena (memory_ == nullptr) is safe.
static void moved_from_arena_destructs_safely() {
    Arena<> source(64);
    {
        Arena<> dest(std::move(source));
        (void)dest;
    } // dest destructs here, releasing the real buffer
    // source destructs at end of scope with memory_ == nullptr
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    CHK(source.capacity() == 0);
}

// Executes all destruction test cases.
static void run_tests() {
    RUN(live_objects_are_not_destructed);
    RUN(moved_from_arena_destructs_safely);
}

REGISTER_TEST_SUITE();
