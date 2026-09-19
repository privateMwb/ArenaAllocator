// Arena create() test suite.
//
// Coverage:
// - Constructs T in place with forwarded arguments
// - Returns nullptr, and constructs nothing, when the allocation fails
// - A throwing constructor propagates the exception

#include <support/framework.h>

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
static void constructs_with_forwarded_args() {
    Arena<> arena(64);
    Point* p = arena.create<Point>(3, 4);
    CHK(p != nullptr);
    CHK(p->x == 3 && p->y == 4);
}

// Verifies create() returns nullptr without constructing anything
// when the underlying allocation cannot be satisfied.
static void returns_nullptr_when_capacity_exhausted() {
    Arena<> arena(1);
    Point* p = arena.create<Point>(1, 2);
    CHK(p == nullptr);
}

// Verifies an exception thrown by T's constructor propagates out of create().
static void throwing_ctor_propagates() {
    Arena<> arena(64);
    CHK_THROWS(arena.create<Thrower>(true), std::runtime_error);
}

// Executes all create() test cases.
static void run_tests() {
    RUN(constructs_with_forwarded_args);
    RUN(returns_nullptr_when_capacity_exhausted);
    RUN(throwing_ctor_propagates);
}

REGISTER_TEST_SUITE();
