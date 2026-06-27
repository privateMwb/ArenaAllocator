// Arena Object Lifecycle Test Suite
// Validates create and destroy behavior for trivial and non-trivial types,
// including constructor forwarding, destructor invocation, and null safety.
//
// Covers:
// - create returns non-null pointer for trivial type
// - create correctly forwards constructor arguments
// - create returns nullptr when arena is full
// - destroy invokes destructor on non-trivial type
// - create and destroy work correctly with Probe type

#include "test_helper.h"

// Create Trivial
// verifies create returns a valid pointer for a trivial type
static void create_trivial() {
    AllocatorPro::Arena arena{1024};
    int* p = arena.create<int>(42);

    CHK(p != nullptr);
    CHK(*p == 42);
}

// Create Forwards Arguments
// verifies create correctly forwards multiple constructor arguments
static void create_forwards_arguments() {
    struct Vec2 {
        float x_, y_;
        Vec2(float x, float y) : x_(x), y_(y) {}
    };

    AllocatorPro::Arena arena{1024};
    Vec2* v = arena.create<Vec2>(1.0f, 2.0f);

    CHK(v != nullptr);
    CHK(v->x_ == 1.0f);
    CHK(v->y_ == 2.0f);
}

// Create Out Of Memory
// verifies create returns nullptr when arena cannot satisfy the allocation
static void create_out_of_memory() {
    AllocatorPro::Arena arena{4};
    double* p = arena.create<double>(3.14);

    CHK(p == nullptr);
}

// Destroy Invokes Destructor
// verifies destroy calls the destructor on a non-trivial type
static void destroy_invokes_destructor() {
    AllocatorPro::Arena arena{1024};
    bool destroyed = false;

    Probe* p = arena.create<Probe>(1, destroyed);
    CHK(p != nullptr);

    arena.destroy(p);
    CHK(destroyed == true);
}

// Create Destroy Probe
// verifies full create/destroy cycle with Probe tracks value and destruction
static void create_destroy_probe() {
    AllocatorPro::Arena arena{1024};
    bool destroyed = false;

    Probe* p = arena.create<Probe>(99, destroyed);

    CHK(p != nullptr);
    CHK(p->value_    == 99);
    CHK(destroyed    == false);

    arena.destroy(p);
    CHK(destroyed == true);
}

// Test Runner
// Executes all object lifecycle test cases.
void run_object_lifecycle_tests() {
    setTitle("Object Lifecycle Tests");

    RUN(create_trivial);
    RUN(create_forwards_arguments);
    RUN(create_out_of_memory);
    RUN(destroy_invokes_destructor);
    RUN(create_destroy_probe);

    std::cout << "\n";
}