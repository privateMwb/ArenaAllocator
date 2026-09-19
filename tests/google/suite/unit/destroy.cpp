// Arena destroy() test suite.
//
// Coverage:
// - Runs the object's destructor
// - Does not return the storage to the arena

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>

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
TEST(Destroy, RunsDestructor) {
    Arena<> arena(64);
    bool destroyed = false;
    Widget* w = arena.create<Widget>(&destroyed);
    ASSERT_NE(w, nullptr);

    arena.destroy(w);
    EXPECT_TRUE(destroyed);
}

// Verifies destroy() leaves the allocation cursor untouched.
TEST(Destroy, DoesNotReclaimStorage) {
    Arena<> arena(64);
    bool destroyed = false;
    Widget* w = arena.create<Widget>(&destroyed);
    const std::size_t usedBefore = arena.used();

    arena.destroy(w);
    EXPECT_EQ(arena.used(), usedBefore);
}
