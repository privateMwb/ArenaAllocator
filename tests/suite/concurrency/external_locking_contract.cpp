// Arena external locking contract test suite.
//
// Coverage:
// - Concurrent allocate() calls, serialized by a caller-supplied
//   mutex, never hand out overlapping blocks and total used()
//   correctly
// - Concurrent beginFrame()/allocate()/endFrame() cycles, serialized
//   the same way, always settle back to an empty, zero-depth arena

#include <algorithm>
#include <mutex>
#include <support/framework.h>
#include <thread>
#include <vector>

using namespace ArenaPro;

// Verifies allocations from many threads, each serialized through the
// same external mutex, never overlap and sum to the expected total.
static void concurrent_allocations_with_external_lock_are_correct() {
    Arena<> arena(4096);
    std::mutex mtx;
    std::vector<std::byte*> pointers;

    constexpr int kThreads = 8;
    constexpr int kAllocsPerThread = 16;
    constexpr std::size_t kAllocSize = 8;

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kAllocsPerThread; ++i) {
                std::lock_guard<std::mutex> lock(mtx);
                // Align to kAllocSize itself so allocations pack tightly —
                // the default alignment (alignof(max_align_t)) would pad
                // every other 8-byte block up to a 16-byte boundary and
                // break the exact-total check below.
                if (std::byte* p = arena.allocate(kAllocSize, kAllocSize))
                    pointers.push_back(p);
            }
        });
    }
    for (auto& th : threads)
        th.join();

    CHK(pointers.size() == static_cast<std::size_t>(kThreads * kAllocsPerThread));

    std::vector<std::byte*> sorted = pointers;
    std::sort(sorted.begin(), sorted.end());
    CHK(std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end());

    CHK(arena.used() == kThreads * kAllocsPerThread * kAllocSize);
}

// Verifies concurrent begin/allocate/end frame cycles, serialized
// through an external mutex, always leave the arena empty afterward.
static void concurrent_frame_cycles_with_external_lock_are_correct() {
    Arena<> arena(4096);
    std::mutex mtx;

    constexpr int kThreads = 8;
    constexpr int kIterations = 20;

    std::vector<std::thread> threads;
    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < kIterations; ++i) {
                std::lock_guard<std::mutex> lock(mtx);
                arena.beginFrame();
                (void)arena.allocate(32);
                arena.endFrame();
            }
        });
    }
    for (auto& th : threads)
        th.join();

    CHK(arena.frameDepth() == 0);
    CHK(arena.used() == 0);
}

// Executes all external locking contract test cases.
static void run_tests() {
    RUN(concurrent_allocations_with_external_lock_are_correct);
    RUN(concurrent_frame_cycles_with_external_lock_are_correct);
}

REGISTER_TEST_SUITE();
