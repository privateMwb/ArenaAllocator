// Arena destruction test suite.
//
// Coverage:
// - Objects created via create() but never destroy()'d are not destructed
//   when the arena itself goes out of scope
// - A moved-from arena (null buffer) destructs safely

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>
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
TEST(Destruction, LiveObjectsAreNotDestructed) {
    bool destroyed = false;
    {
        Arena<> arena(64);
        (void)arena.create<Widget>(&destroyed);
    } // arena destructs here
    EXPECT_FALSE(destroyed);
}

// Verifies destructing a moved-from arena (memory_ == nullptr) is safe.
TEST(Destruction, MovedFromArenaDestructsSafely) {
    Arena<> source(64);
    {
        Arena<> dest(std::move(source));
        (void)dest;
    } // dest destructs here, releasing the real buffer
    // source destructs at end of scope with memory_ == nullptr
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.Move)
    EXPECT_EQ(source.capacity(), 0u);
}
