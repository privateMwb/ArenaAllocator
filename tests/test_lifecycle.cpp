// Arena Object Lifecycle Test Suite
// Validates object construction and destruction within the arena,
// including constructor forwarding, allocation failure, and
// destructor invocation.
//
// Covers:
// - trivial object creation
// - constructor argument forwarding
// - out-of-memory object creation
// - destructor invocation
// - complete create/destroy lifecycle

#include "test_helper.h"

// Probe Type
// Tracks object destruction for lifecycle verification.
struct Probe {
    int   value_;
    bool& destroyed_;

    Probe(int value, bool& destroyed)
        : value_(value), destroyed_(destroyed) {}

    ~Probe() { destroyed_ = true; }
};

// Verifies trivial objects are constructed successfully.
static void create_trivial() {
    AllocatorPro::Arena<false> arena{1024};
    int* p = arena.create<int>(42);

    CHK(p != nullptr);
    CHK(*p == 42);
}

// Verifies constructor arguments are perfectly forwarded.
static void create_forwards_arguments() {
    struct Vec2 {
        float x_, y_;
        Vec2(float x, float y) : x_(x), y_(y) {}
    };

    AllocatorPro::Arena<false> arena{1024};
    Vec2* v = arena.create<Vec2>(1.0f, 2.0f);

    CHK(v != nullptr);
    CHK(v->x_ == 1.0f);
    CHK(v->y_ == 2.0f);
}

// Verifies object creation fails when the arena lacks capacity.
static void create_out_of_memory() {
    AllocatorPro::Arena<false> arena{4};
    double* p = arena.create<double>(3.14);

    CHK(p == nullptr);
}

// Verifies destroy invokes the object's destructor.
static void destroy_invokes_destructor() {
    AllocatorPro::Arena<false> arena{1024};
    bool destroyed = false;

    Probe* p = arena.create<Probe>(1, destroyed);
    CHK(p != nullptr);

    arena.destroy(p);
    CHK(destroyed == true);
}

// Verifies objects complete the full create/destroy lifecycle.
static void create_destroy_probe() {
    AllocatorPro::Arena<false> arena{1024};
    bool destroyed = false;

    Probe* p = arena.create<Probe>(99, destroyed);

    CHK(p != nullptr);
    CHK(p->value_ == 99);
    CHK(destroyed == false);

    arena.destroy(p);
    CHK(destroyed == true);
}

// Executes all object lifecycle test cases.
void run_lifecycle_tests() {
    setTitle("Object Lifecycle Tests");

    RUN(create_trivial);
    RUN(create_forwards_arguments);
    RUN(create_out_of_memory);
    RUN(destroy_invokes_destructor);
    RUN(create_destroy_probe);

    std::cout << "\n";
}