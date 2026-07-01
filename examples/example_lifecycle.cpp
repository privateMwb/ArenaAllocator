// Example Lifecycle
//
// Covers:
// - Trivial object creation and destruction
// - Non-trivial object creation and destruction
// - Multiple objects in a single arena
// - ArenaScope automatic rollback

#include "example_helper.h"

using namespace AllocatorPro;

namespace {

struct Vec3 {
    float x_, y_, z_;
    Vec3(float x, float y, float z) : x_(x), y_(y), z_(z) {}
    ~Vec3() = default;
};

struct Node {
    int   id_;
    float weight_;
    bool  active_;

    Node(int id, float weight, bool active)
        : id_(id), weight_(weight), active_(active) {}
    ~Node() = default;
};

} // namespace

int main() {
    mainTitle("\nObject Lifecycle Examples");
    borderLine();

    Arena<false> arena{4096};

    // Creates and destroys a trivially destructible object.
    setTitle("Trivial Type");

    int* counter = arena.create<int>(0);
    *counter = 42;

    std::cout << "int value   : " << *counter << "\n";
    std::cout << "Owns ptr    : " << arena.owns(counter) << "\n";

    arena.destroy(counter);
    std::cout << "Destroyed   : true\n\n";

    // Creates and destroys a user-defined object.
    setTitle("Non-Trivial Type");

    Vec3* v = arena.create<Vec3>(1.0f, 2.0f, 3.0f);

    std::cout << "Vec3.x : " << v->x_ << "\n";
    std::cout << "Vec3.y : " << v->y_ << "\n";
    std::cout << "Vec3.z : " << v->z_ << "\n";

    arena.destroy(v);
    std::cout << "Destroyed : true\n\n";

    // Allocates multiple objects from the same arena.
    setTitle("Multiple Objects");

    arena.reset();

    Node* n1 = arena.create<Node>(1, 0.5f, true);
    Node* n2 = arena.create<Node>(2, 1.2f, false);
    Node* n3 = arena.create<Node>(3, 0.8f, true);

    std::cout << "Node 1 — id: " << n1->id_ << "  weight: " << n1->weight_ << "  active: " << n1->active_ << "\n";
    std::cout << "Node 2 — id: " << n2->id_ << "  weight: " << n2->weight_ << "  active: " << n2->active_ << "\n";
    std::cout << "Node 3 — id: " << n3->id_ << "  weight: " << n3->weight_ << "  active: " << n3->active_ << "\n";

    std::cout << "Used after 3 nodes : " << arena.used() << "\n\n";

    arena.destroy(n1);
    arena.destroy(n2);
    arena.destroy(n3);

    // Uses ArenaScope to automatically roll back temporary allocations.
    setTitle("ArenaScope");

    arena.reset();

    Vec3* persistent = arena.create<Vec3>(0.0f, 1.0f, 0.0f);
    (void)persistent;

    std::cout << "Used before scope : " << arena.used() << "\n";
    std::cout << "Frame depth       : " << arena.frameDepth() << "\n";

    {
        ArenaScope<false> scope{arena};

        Vec3* temp1 = arena.create<Vec3>(1.0f, 0.0f, 0.0f);
        Vec3* temp2 = arena.create<Vec3>(0.0f, 0.0f, 1.0f);
        (void)temp1;
        (void)temp2;

        std::cout << "Used inside scope : " << arena.used() << "\n";
        std::cout << "Frame depth       : " << arena.frameDepth() << "\n";
    }

    std::cout << "Used after scope  : " << arena.used() << "\n";
    std::cout << "Frame depth       : " << arena.frameDepth() << "\n";

    borderLine();
    std::cout << "\n";
    return 0;
}