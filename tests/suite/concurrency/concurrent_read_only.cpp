// Arena concurrent read-only test suite.
//
// Coverage:
// - The const introspection methods (owns(), used(), remaining(),
//   capacity(), frameDepth(), getStats()) return consistent values
//   when called from many threads with no external lock, provided no
//   writer is active
// - view() returns a consistently-sized span under the same conditions

#include <atomic>
#include <support/framework.h>
#include <thread>
#include <vector>

using namespace ArenaPro;

// Verifies every read-only observer stays consistent across
// concurrent, lock-free callers once the arena's state is settled.
static void concurrent_reads_return_consistent_values() {
    Arena<true> arena(1024);
    std::byte* block = arena.allocate(64);

    constexpr int kThreads = 8;
    constexpr int kIterations = 100;
    std::atomic<bool> allConsistent{true};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kIterations; ++i) {
                const bool ownsIt = arena.owns(block);
                const std::size_t used = arena.used();
                const std::size_t remaining = arena.remaining();
                const std::size_t cap = arena.capacity();
                const std::size_t depth = arena.frameDepth();
                const auto& stats = arena.getStats();

                const bool ok = ownsIt && used == 64 && cap == 1024 && remaining == cap - used &&
                                depth == 0 && stats.totalAllocated_ == 64;
                if (!ok)
                    allConsistent.store(false, std::memory_order_relaxed);
            }
        });
    }
    for (auto& th : threads)
        th.join();

    CHK(allConsistent.load());
}

// Verifies view() reports the same span size to every concurrent,
// lock-free caller once the arena's state is settled.
static void concurrent_view_calls_are_consistent() {
    Arena<> arena(256);
    (void)arena.allocate(50);

    constexpr int kThreads = 8;
    constexpr int kIterations = 100;
    std::atomic<bool> allConsistent{true};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kIterations; ++i) {
                if (arena.view().size() != 50)
                    allConsistent.store(false, std::memory_order_relaxed);
            }
        });
    }
    for (auto& th : threads)
        th.join();

    CHK(allConsistent.load());
}

// Executes all concurrent read-only test cases.
static void run_tests() {
    RUN(concurrent_reads_return_consistent_values);
    RUN(concurrent_view_calls_are_consistent);
}

REGISTER_TEST_SUITE();
