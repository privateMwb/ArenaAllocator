// example_scope.cpp
// Demonstrates ArenaScope for automatic frame management
// across nested scopes and multiple temporary allocation groups.

#include "example_helper.h"

#include <cstddef>

using namespace AllocatorPro;

struct Particle {
    float x_, y_, z_;
    float lifetime_;

    Particle(float x, float y, float z, float lifetime)
        : x_(x), y_(y), z_(z), lifetime_(lifetime) {}
};

int main() {
    mainTitle("\nScope Examples");
    borderLine();
    
    Arena arena{4096};

    // Persistent Allocation
    setTitle("Persistent Allocation");

    int* persistent = arena.create<int>(42);
    (void)persistent;

    std::cout << "Used after persistent alloc : " << arena.used() << "\n\n";

    // Scoped Allocation
    setTitle("Scoped Allocation");

    std::cout << "Used before scope    : " << arena.used() << "\n";

    {
        ArenaScope scope{arena};

        Particle* p1 = arena.create<Particle>(1.0f, 2.0f, 3.0f, 5.0f);
        Particle* p2 = arena.create<Particle>(4.0f, 5.0f, 6.0f, 3.0f);
        Particle* p3 = arena.create<Particle>(7.0f, 8.0f, 9.0f, 1.0f);
        (void)p1;
        (void)p2;
        (void)p3;

        std::cout << "Used inside scope    : " << arena.used()    << "\n";
        std::cout << "Particle 1 lifetime  : " << p1->lifetime_   << "\n";
        std::cout << "Particle 2 lifetime  : " << p2->lifetime_   << "\n";
        std::cout << "Particle 3 lifetime  : " << p3->lifetime_   << "\n";
    }

    std::cout << "Used after scope     : " << arena.used() << "\n\n";

    // Nested Scopes
    setTitle("Nested Scopes");

    std::cout << "Used before outer scope      : " << arena.used() << "\n";

    {
        ArenaScope outer{arena};

        Particle* wave1 = arena.create<Particle>(0.0f, 0.0f, 0.0f, 10.0f);
        (void)wave1;

        std::cout << "Used after outer alloc       : " << arena.used() << "\n";

        {
            ArenaScope inner{arena};

            Particle* wave2 = arena.create<Particle>(1.0f, 1.0f, 1.0f, 2.0f);
            (void)wave2;

            std::cout << "Used inside inner scope      : " << arena.used() << "\n";
        }

        std::cout << "Used after inner scope exits : " << arena.used() << "\n";
    }

    std::cout << "Used after outer scope exits : " << arena.used() << "\n\n";

    // Final Stats
    setTitle("Final Stats");

    const auto& s = arena.getStats();

    std::cout << "Total allocated : " << s.totalAllocated_ << "\n";
    std::cout << "Peak used       : " << s.peakUsed_       << "\n";
    std::cout << "Allocations     : " << s.allocations_    << "\n";
    
    borderLine();
    std::cout << "\n";

    return 0;
}

