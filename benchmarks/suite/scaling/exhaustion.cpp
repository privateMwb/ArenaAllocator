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
// Covers:
// - allocate() with room to spare (every call succeeds)
// - allocate() with no room left (every call fails)

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kCapacityBytes = 1024 * 1024;
constexpr std::size_t kAllocSize = sizeof(int);
} // namespace

// Measures allocate() with plenty of room left — the ordinary
// success path. Wrapped in beginFrame()/endFrame() so the cursor never
// advances cumulatively, for the same reason as capacity_growth.cpp.
static void bench_allocate_success() {
    Arena<false> cSrc(kCapacityBytes);
    // Left unbounded (default upstream) deliberately: this case is not
    // about exhaustion, so stdArena is left free to do what it always
    // does when it needs more room — grow — rather than being reset
    // every call, which would load the measurement with reset
    // overhead that has nothing to do with allocate() itself.
    stdArena sSrc(kCapacityBytes);

    auto c = [&] {
        cSrc.beginFrame();
        doNotOptimize(cSrc.allocate(kAllocSize));
        cSrc.endFrame();
    };

    auto s = [&] { doNotOptimize(sSrc.allocate(kAllocSize)); };

    BENCH("allocate() with room to spare", c, s);
}

// Measures allocate() with no room left — every call fails. Arena
// returns nullptr, noexcept; the bounded stdArena has no choice but to
// throw, so its side must catch std::bad_alloc every call.
static void bench_allocate_failure() {
    Arena<false> cSrc(kCapacityBytes);
    doNotOptimize(cSrc.allocate(cSrc.remaining()));
    // cSrc now has exactly 0 bytes remaining.

    alignas(std::max_align_t) static std::byte backing[kCapacityBytes];
    stdArena sSrc(backing, sizeof(backing), std::pmr::null_memory_resource());
    doNotOptimize(sSrc.allocate(kCapacityBytes));
    // sSrc now has exactly 0 bytes remaining, with no upstream to fall
    // back on.

    auto c = [&] {
        std::byte* p = cSrc.allocate(kAllocSize);
        doNotOptimize(p);
    };

    auto s = [&] {
        try {
            void* p = sSrc.allocate(kAllocSize);
            doNotOptimize(p);
        } catch (const std::bad_alloc&) {
            // Expected on every call — the arena is full.
        }
    };

    BENCH("allocate() at capacity (failure path)", c, s);
}

// Executes all exhaustion benchmark cases.
static void run_benchmarks() {
    bench_allocate_success();
    std::cout << "\n";

    bench_allocate_failure();
}

REGISTER_BENCH_SUITE();
