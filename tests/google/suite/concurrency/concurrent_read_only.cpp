// Arena concurrent read-only test suite.
//
// Coverage:
// - The const introspection methods (owns(), used(), remaining(),
//   capacity(), frameDepth(), getStats()) return consistent values
//   when called from many threads with no external lock, provided no
//   writer is active
// - view() returns a consistently-sized span under the same conditions

#include <ArenaPro/Arena.h>
#include <ArenaPro/ArenaScope.h>
#include <atomic>
#include <cstddef>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace ArenaPro;

// Verifies every read-only observer stays consistent across
// concurrent, lock-free callers once the arena's state is settled.
TEST(ConcurrentReadOnly, ReadsConsistentValues) {
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

    EXPECT_TRUE(allConsistent.load());
}

// Verifies view() reports the same span size to every concurrent,
// lock-free caller once the arena's state is settled.
TEST(ConcurrentReadOnly, ViewCallsConsistent) {
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

    EXPECT_TRUE(allConsistent.load());
}
