// Arena Lifecycle Benchmark Suite — Move
// Measures Arena move-construct and move-assign performance.
//
// std::pmr::monotonic_buffer_resource has both its move constructor
// and move assignment operator explicitly deleted — it isn't movable
// at all — so there's nothing to pair against. Runs solo.
//
// Covers:
// - move-construction from a populated arena
// - move-assignment, ping-ponged between two populated arenas

#include <support/framework.h>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures move-construction from a populated source arena. The
// source is rebuilt after every call — a moved-from Arena has nothing
// left to move out of, so this cannot repeat on the same object like
// a pure read can.
static void bench_move_construct() {
    auto a = [&] {
        Arena<false> src(kSize);
        doNotOptimize(src.allocate(64));
        Arena<false> dst(std::move(src));
        doNotOptimize(dst);
    };

    BENCH_SOLO("move construct", a);
}

// Measures move-assignment, ping-ponged between two populated arenas
// so every call has a real (non-empty) source and destination to move
// between, not a degenerate already-moved-from one.
static void bench_move_assign() {
    Arena<false> a1(kSize);
    Arena<false> a2(kSize);
    doNotOptimize(a1.allocate(64));
    doNotOptimize(a2.allocate(64));

    bool flip = false;
    auto a = [&] {
        if (flip)
            a1 = std::move(a2);
        else
            a2 = std::move(a1);
        flip = !flip;
    };

    BENCH_SOLO("move assign", a);
}

// Executes all move benchmark cases.
static void run_benchmarks() {
    bench_move_construct();
    std::cout << "\n";

    bench_move_assign();
}

REGISTER_BENCH_SUITE();
