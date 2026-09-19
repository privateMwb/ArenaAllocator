// Arena create/destroy cycle integration test suite.
//
// Coverage:
// - destroy() runs the destructor but does not reclaim storage;
//   only reset() actually frees the bytes
// - An object created after reset() reuses the bytes an earlier,
//   destroyed object occupied

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

// Verifies destroy() alone leaves used() unchanged, while reset() reclaims it.
TEST(CreateDestroyCycle, DestroyNoFreeResetDoes) {
    Arena<> arena(64);
    bool destroyed = false;

    Widget* w = arena.create<Widget>(&destroyed);
    const std::size_t usedAfterCreate = arena.used();

    arena.destroy(w);
    EXPECT_TRUE(destroyed);
    EXPECT_EQ(arena.used(), usedAfterCreate);

    arena.reset();
    EXPECT_EQ(arena.used(), 0u);
}

// Verifies an object created after reset() lands on the same bytes an
// earlier, destroyed object used.
TEST(CreateDestroyCycle, ObjectAfterResetReusesBytes) {
    Arena<> arena(64);
    bool destroyed = false;

    Widget* first = arena.create<Widget>(&destroyed);
    arena.destroy(first);
    arena.reset();

    bool destroyedAgain = false;
    Widget* second = arena.create<Widget>(&destroyedAgain);

    EXPECT_EQ(reinterpret_cast<std::byte*>(first), reinterpret_cast<std::byte*>(second));
}
