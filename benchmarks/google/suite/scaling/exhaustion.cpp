// Arena Scaling Benchmark Suite — Exhaustion
// Measures Arena allocate() cost as the arena approaches and then
// passes full capacity, against stdArena.
//
// A bump allocator's cost is position-independent — there's no search,
// no fragmentation, so a successful call near the end of the buffer
// costs the same as one at the start. The one place cost genuinely
// changes is the failure path itself, so that's what this file
// isolates: a case with room to spare (every call succeeds) against a
// case that's already full (every call fails).
//
// A default-constructed stdArena can't fail this way — once its
// initial buffer runs out, it transparently pulls another block from
// its upstream resource instead of failing. To get a stdArena that
// actually fails at a fixed capacity, this file builds one over a
// fixed external buffer with std::pmr::null_memory_resource() as its
// upstream, which throws std::bad_alloc instead of granting more
// memory. That also means the two failure paths aren't symmetric:
// Arena signals exhaustion with a noexcept nullptr return, while a
// bounded stdArena can only signal it by throwing — so the "failure"
// case below is also, in part, a measurement of exception-handling
// overhead versus a plain branch. That asymmetry is real and worth
// seeing, not an artifact to hide.
//
// The success cases run a fixed kIterations on both sides, so the two
// are compared over identical work and stdArena's upstream growth
// stays bounded. The failure cases never change the arena's state, so
// their iteration counts are left to Google Benchmark.
//
// Covers:
// - allocate() with room to spare (every call succeeds)
// - allocate() with no room left (every call fails)

#include <support/framework.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <memory_resource>
#include <new>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
constexpr std::size_t kIterations = 1'000'000;
} // namespace

// Measures Arena allocate() with plenty of room left — the ordinary
// success path. Wrapped in beginFrame()/endFrame() so the cursor never
// advances cumulatively, for the same reason as capacity_growth.cpp.
static void AllocateSuccessArena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);

    for (auto _ : state) {
        cSrc.beginFrame();
        benchmark::DoNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    }
}
BENCHMARK(AllocateSuccessArena)->Iterations(kIterations);

// Measures stdArena allocate() with plenty of room left.
// Left unbounded (default upstream) deliberately: this case is not
// about exhaustion, so stdArena is left free to do what it always
// does when it needs more room — grow — rather than being reset
// every call, which would load the measurement with reset
// overhead that has nothing to do with allocate() itself.
static void AllocateSuccessStd(benchmark::State& state) {
    stdArena sSrc(kCapacityBytes);

    for (auto _ : state)
        benchmark::DoNotOptimize(sSrc.allocate(kAllocSize));
}
BENCHMARK(AllocateSuccessStd)->Iterations(kIterations);

// Measures Arena allocate() with no room left — every call fails and
// returns nullptr, noexcept.
static void AllocateFailureArena(benchmark::State& state) {
    Arena<false> cSrc(kCapacityBytes);
    benchmark::DoNotOptimize(cSrc.allocate(cSrc.remaining()));
    // cSrc now has exactly 0 bytes remaining.

    for (auto _ : state) {
        std::byte* p = cSrc.allocate(kAllocSize);
        benchmark::DoNotOptimize(p);
    }
}
BENCHMARK(AllocateFailureArena);

// Measures the bounded stdArena with no room left — every call fails.
// The bounded stdArena has no choice but to throw, so this side must
// catch std::bad_alloc every call.
static void AllocateFailureStd(benchmark::State& state) {
    alignas(std::max_align_t) static std::byte backing[kCapacityBytes];
    stdArena sSrc(backing, sizeof(backing), std::pmr::null_memory_resource());
    benchmark::DoNotOptimize(sSrc.allocate(kCapacityBytes));
    // sSrc now has exactly 0 bytes remaining, with no upstream to fall
    // back on.

    for (auto _ : state) {
        try {
            void* p = sSrc.allocate(kAllocSize);
            benchmark::DoNotOptimize(p);
        } catch (const std::bad_alloc&) {
            // Expected on every call — the arena is full.
        }
    }
}
BENCHMARK(AllocateFailureStd);
