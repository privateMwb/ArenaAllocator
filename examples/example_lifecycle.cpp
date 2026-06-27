// example_lifecycle.cpp
// Demonstrates object creation and destruction with non-trivial types,
// and frame-based allocation for temporary object groups.

#include "example_helper.h"

#include <cstddef>

using namespace AllocatorPro;

struct Entity {
    int         id_;
    float       health_;
    const char* name_;

    Entity(int id, float health, const char* name)
        : id_(id), health_(health), name_(name) {}

    ~Entity() {}
};

int main() {
    mainTitle("\nLifecycle Examples");
    borderLine();
    
    Arena arena{2048};

    // Object Creation
    setTitle("Object Creation");

    Entity* e1 = arena.create<Entity>(1, 100.0f, "Warrior");
    Entity* e2 = arena.create<Entity>(2, 80.0f,  "Mage");
    Entity* e3 = arena.create<Entity>(3, 60.0f,  "Rogue");

    std::cout << "Entity 1          : " << e1->name_ << "\n";
    std::cout << "Entity 2          : " << e2->name_ << "\n";
    std::cout << "Entity 3          : " << e3->name_ << "\n";
    std::cout << "Used after create : " << arena.used() << "\n\n";

    // Object Destruction
    setTitle("Object Destruction");

    arena.destroy(e3);

    std::cout << "Used after destroy e3 : " << arena.used()     << "\n";
    std::cout << "Capacity preserved    : " << arena.capacity() << "\n\n";

    // Frame Allocation
    setTitle("Frame Allocation");

    std::cout << "Used before frame   : " << arena.used() << "\n";

    arena.beginFrame();

    Entity* temp1 = arena.create<Entity>(10, 50.0f, "Temp Goblin");
    Entity* temp2 = arena.create<Entity>(11, 30.0f, "Temp Orc");
    (void)temp1;
    (void)temp2;

    std::cout << "Used inside frame   : " << arena.used()   << "\n";
    std::cout << "Temp entity 1       : " << temp1->name_   << "\n";
    std::cout << "Temp entity 2       : " << temp2->name_   << "\n";

    arena.endFrame();

    std::cout << "Used after endFrame : " << arena.used() << "\n\n";

    // Stats
    setTitle("Stats");

    const auto& s = arena.getStats();

    std::cout << "Total allocated : " << s.totalAllocated_ << "\n";
    std::cout << "Peak used       : " << s.peakUsed_       << "\n";
    std::cout << "Allocations     : " << s.allocations_    << "\n";
    
    borderLine();
    std::cout << "\n";

    return 0;
}

