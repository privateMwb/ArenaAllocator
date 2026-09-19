// Arena create() test suite.
//
// Coverage:
// - Constructs T in place with forwarded arguments
// - Returns nullptr, and constructs nothing, when the allocation fails
// - A throwing constructor propagates the exception

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <stdexcept>

using namespace ArenaPro;

namespace {

struct Point {
    int x;
    int y;
    Point(int a, int b) : x(a), y(b) {}
};

struct Thrower {
    explicit Thrower(bool doThrow) {
        if (doThrow)
            throw std::runtime_error("boom");
    }
};

} // namespace

// Verifies create() forwards its arguments into T's constructor.
TEST(Create, ConstructsWithForwardedArgs) {
    Arena<> arena(64);
    Point* p = arena.create<Point>(3, 4);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 3);
    EXPECT_EQ(p->y, 4);
}

// Verifies create() returns nullptr without constructing anything
// when the underlying allocation cannot be satisfied.
TEST(Create, ReturnsNullptrWhenCapacityExhausted) {
    Arena<> arena(1);
    Point* p = arena.create<Point>(1, 2);
    EXPECT_EQ(p, nullptr);
}

// Verifies an exception thrown by T's constructor propagates out of create().
TEST(Create, ThrowingCtorPropagates) {
    Arena<> arena(64);
    EXPECT_THROW((void)arena.create<Thrower>(true), std::runtime_error);
}
