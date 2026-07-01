// Example Scope
//
// Covers:
// - Basic ArenaScope usage
// - Nested ArenaScope rollback
// - Sequential scopes in the same arena
// - ArenaScope with object lifecycle

#include "example_helper.h"

using namespace AllocatorPro;

namespace {

struct Particle {
    float x_, y_, z_;
    float lifetime_;

    Particle(float x, float y, float z, float lifetime)
        : x_(x), y_(y), z_(z), lifetime_(lifetime) {}
};

} // namespace

int main() {
    mainTitle("\nArenaScope Examples");
    borderLine();

    Arena<false> arena{4096};

    // Demonstrates how ArenaScope automatically restores the arena
    // to its previous state when leaving scope.
    setTitle("Basic Scope");

    (void)arena.allocate(64, alignof(std::max_align_t));

    std::cout << "Used before scope  : " << arena.used()       << "\n";
    std::cout << "Frame depth before : " << arena.frameDepth() << "\n";

    {
        ArenaScope<false> scope{arena};

        (void)arena.allocate(256, alignof(std::max_align_t));
        (void)arena.allocate(128, alignof(std::max_align_t));

        std::cout << "Used inside scope  : " << arena.used()       << "\n";
        std::cout << "Frame depth inside : " << arena.frameDepth() << "\n";
    }

    std::cout << "Used after scope   : " << arena.used()       << "\n";
    std::cout << "Frame depth after  : " << arena.frameDepth() << "\n\n";

    // Shows that nested scopes restore memory independently,
    // unwinding in reverse order.
    setTitle("Nested Scopes");

    arena.reset();

    (void)arena.allocate(64, alignof(std::max_align_t));
    const std::size_t base = arena.used();

    std::cout << "Used at base             : " << base               << "\n";

    {
        ArenaScope<false> outer{arena};
        (void)arena.allocate(256, alignof(std::max_align_t));
        const std::size_t mid = arena.used();

        std::cout << "Used inside outer scope  : " << mid                << "\n";
        std::cout << "Frame depth (outer open) : " << arena.frameDepth() << "\n";

        {
            ArenaScope<false> inner{arena};
            (void)arena.allocate(512, alignof(std::max_align_t));

            std::cout << "Used inside inner scope  : " << arena.used()       << "\n";
            std::cout << "Frame depth (inner open) : " << arena.frameDepth() << "\n";
        }

        std::cout << "Used after inner closed  : " << arena.used()       << "\n";
        std::cout << "Frame depth (inner done) : " << arena.frameDepth() << "\n";
    }

    std::cout << "Used after outer closed  : " << arena.used()       << "\n";
    std::cout << "Frame depth (outer done) : " << arena.frameDepth() << "\n\n";

    // Demonstrates that independent scopes can be reused without
    // affecting allocations outside their lifetime.
    setTitle("Sequential Scopes");

    arena.reset();

    std::cout << "Frame depth at start : " << arena.frameDepth() << "\n";

    {
        ArenaScope<false> first{arena};
        (void)arena.allocate(128, alignof(std::max_align_t));
        std::cout << "Used inside first    : " << arena.used() << "\n";
    }
    std::cout << "Used after first     : " << arena.used() << "\n";

    {
        ArenaScope<false> second{arena};
        (void)arena.allocate(256, alignof(std::max_align_t));
        std::cout << "Used inside second   : " << arena.used() << "\n";
    }
    std::cout << "Used after second    : " << arena.used() << "\n";

    {
        ArenaScope<false> third{arena};
        (void)arena.allocate(512, alignof(std::max_align_t));
        std::cout << "Used inside third    : " << arena.used() << "\n";
    }
    std::cout << "Used after third     : " << arena.used() << "\n";
    std::cout << "Frame depth at end   : " << arena.frameDepth() << "\n\n";

    // Shows that ArenaScope rolls back temporary allocations while
    // preserving objects allocated before the scope.
    setTitle("Scope With Object Lifecycle");

    arena.reset();

    Particle* persistent = arena.create<Particle>(0.0f, 0.0f, 0.0f, -1.0f);
    (void)persistent;

    std::cout << "Used before scope (persistent particle) : " << arena.used() << "\n";

    {
        ArenaScope<false> scope{arena};

        Particle* p1 = arena.create<Particle>(1.0f, 0.0f, 0.0f, 2.5f);
        Particle* p2 = arena.create<Particle>(0.0f, 1.0f, 0.0f, 1.0f);
        Particle* p3 = arena.create<Particle>(0.0f, 0.0f, 1.0f, 0.5f);
        (void)p1;
        (void)p2;
        (void)p3;

        std::cout << "Temp particles created               : " << 3            << "\n";
        std::cout << "Used inside scope                    : " << arena.used() << "\n";

        arena.destroy(p1);
        arena.destroy(p2);
        arena.destroy(p3);
    }

    std::cout << "Used after scope (persistent remains): " << arena.used() << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}