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

#include <benchmark/benchmark.h>

#include <cstddef>
#include <utility>

using namespace ArenaPro;

namespace {
constexpr std::size_t kSize = 4096;
} // namespace

// Measures move-construction from a populated source arena. The
// source is rebuilt on every iteration — a moved-from Arena has nothing
// left to move out of, so this cannot repeat on the same object like
// a pure read can.
static void move_construct(benchmark::State& state) {
    for (auto _ : state) {
        Arena<false> src(kSize);
        benchmark::DoNotOptimize(src.allocate(64));
        Arena<false> dst(std::move(src));
        benchmark::DoNotOptimize(dst);
    }
}
BENCHMARK(move_construct);

// Measures move-assignment, ping-ponged between two populated arenas
// so every iteration has a real (non-empty) source and destination to
// move between, not a degenerate already-moved-from one.
static void move_assign(benchmark::State& state) {
    Arena<false> a1(kSize);
    Arena<false> a2(kSize);
    benchmark::DoNotOptimize(a1.allocate(64));
    benchmark::DoNotOptimize(a2.allocate(64));

    bool flip = false;
    for (auto _ : state) {
        if (flip)
            a1 = std::move(a2);
        else
            a2 = std::move(a1);
        flip = !flip;
    }
}
BENCHMARK(move_assign);
