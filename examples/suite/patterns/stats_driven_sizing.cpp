// Sizing an arena using its own statistics.
//
// Demonstrates:
// - Running a representative workload against an oversized arena
// - Reading peakUsed_ back out via getStats()
// - Using that number to pick a tighter capacity for production

#include <support/framework.h>

using namespace ArenaPro;

namespace {

void simulateWorkload(Arena<true>& arena) {
    ArenaScope<true> scope(arena);
    for (int i = 0; i < 20; ++i) {
        (void)arena.allocate(32);
    }
}

} // namespace

static void run_examples() {

    setTitle("An Oversized Arena");

    // Start generously sized while the real usage pattern is unknown.
    Arena<true> arena(1 << 20);

    simulateWorkload(arena);

    const auto& stats = arena.getStats();
    std::cout << "peakUsed after workload: " << stats.peakUsed_ << "\n";
    std::cout << "capacity available     : " << arena.capacity() << "\n\n";

    setTitle("Picking A Tighter Capacity");

    // A production arena can be sized off peakUsed_ plus headroom,
    // instead of guessing or over-provisioning by default.
    std::size_t recommended = stats.peakUsed_ + stats.peakUsed_ / 4; // 25% headroom
    std::cout << "recommended capacity: " << recommended << "\n";
}

REGISTER_EXAMPLE_SUITE();
