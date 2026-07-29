// One arena per thread.
//
// Demonstrates:
// - Arena has no internal locking, so a single instance isn't safe to
//   share across threads
// - Giving each thread its own arena instead of synchronizing access
//   to a shared one
// - Each thread's allocations and rollbacks staying fully independent

#include <support/framework.h>

#include <thread>
#include <vector>

using namespace ArenaPro;

namespace {

void worker(int id, std::size_t& out_used) {
    // Each thread constructs and owns its own arena — no shared state,
    // so no synchronization is needed for any of this.
    Arena<> arena(4096);

    for (int i = 0; i < id + 1; ++i) {
        (void)arena.allocate(64);
    }

    out_used = arena.used();
}

} // namespace

static void run_examples() {

    setTitle("Four Independent Arenas");

    constexpr int kThreadCount = 4;
    std::vector<std::size_t> used(kThreadCount);
    std::vector<std::thread> threads;

    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back(worker, i, std::ref(used[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    for (int i = 0; i < kThreadCount; ++i) {
        std::cout << "thread " << i << " used: " << used[i] << " bytes\n";
    }
}

REGISTER_EXAMPLE_SUITE();
