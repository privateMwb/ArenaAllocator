// Bulk allocation of same-type structs.
//
// Demonstrates:
// - create<T>() in a loop to fill an arena with many small objects
// - Tracking how much of the arena a batch consumes
// - The point at which allocation starts failing, and handling it
//   cleanly

#include <support/framework.h>

using namespace ArenaPro;

namespace {
struct Particle {
    float x, y, z;
};
} // namespace

static void run_examples() {

    setTitle("Filling An Arena With Particles");

    Arena<> arena(1024);

    int created = 0;
    while (arena.create<Particle>(0.0f, 0.0f, 0.0f) != nullptr) {
        ++created;
    }

    std::cout << "particles created: " << created << "\n";
    std::cout << "sizeof(Particle) : " << sizeof(Particle) << "\n";
    std::cout << "used             : " << arena.used() << "\n";
    std::cout << "remaining        : " << arena.remaining() << "\n\n";

    setTitle("Why It Stopped There");

    // The loop above ends the moment create<Particle>() can't satisfy the
    // next request — remaining() is smaller than sizeof(Particle), so
    // the last create<T>() call returns nullptr and the loop condition
    // fails.
    std::cout << "remaining < sizeof(Particle): " << (arena.remaining() < sizeof(Particle)) << "\n";
}

REGISTER_EXAMPLE_SUITE();
