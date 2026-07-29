// Arena scope-per-thread test suite.
//
// Coverage:
// - Each thread owning its own Arena, wrapped in its own ArenaScope,
//   observes only its own state — independently-owned arenas running
//   concurrently don't leak state into one another

#include <ArenaPro/ArenaScope.h>
#include <atomic>
#include <support/framework.h>
#include <thread>
#include <vector>

using namespace ArenaPro;

// Verifies each thread's own arena/scope pair behaves correctly while
// every other thread is doing the same thing concurrently.
static void independent_arenas_do_not_interfere_across_threads() {
    constexpr int kThreads = 8;
    std::atomic<bool> allConsistent{true};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            Arena<> arena(256);
            ArenaScope<false> scope(arena);

            std::byte* p = arena.allocate(32);
            const bool ok = p != nullptr && arena.used() == 32 && arena.frameDepth() == 1;
            if (!ok)
                allConsistent.store(false, std::memory_order_relaxed);
        });
    }
    for (auto& th : threads)
        th.join();

    CHK(allConsistent.load());
}

// Verifies each thread's arena reports its own distinct capacity,
// unaffected by the other arenas running at the same time.
static void each_thread_owns_independent_capacity() {
    constexpr int kThreads = 8;
    std::atomic<bool> allConsistent{true};

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t]() {
            const std::size_t capacity = 64 * static_cast<std::size_t>(t + 1);
            Arena<> arena(capacity);
            ArenaScope<false> scope(arena);
            (void)arena.allocate(16);

            if (arena.capacity() != capacity)
                allConsistent.store(false, std::memory_order_relaxed);
        });
    }
    for (auto& th : threads)
        th.join();

    CHK(allConsistent.load());
}

// Executes all scope-per-thread test cases.
static void run_tests() {
    RUN(independent_arenas_do_not_interfere_across_threads);
    RUN(each_thread_owns_independent_capacity);
}

REGISTER_TEST_SUITE();
